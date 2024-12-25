package main

/*
#include <stdlib.h>
*/
import "C"
import (
	"encoding/hex"
	"encoding/json"
	"fmt"
	"log"
	"time"

	"github.com/nsqio/go-nsq"
)

var (
	messageChans = make(map[string]chan *nsq.Message) // 主题+通道 -> 消息通道
	messageStore = make(map[string]*nsq.Message)      // 主题+通道 -> 消息通道
	consumers    = make(map[string]*nsq.Consumer)
)

// 处理接收到的消息
type MessageHandler struct {
	topicChan    string
	authResponse uint16
}

func (h *MessageHandler) HandleMessage(m *nsq.Message) error {
	messageStore[hex.EncodeToString(m.ID[:])] = m
	if h.authResponse == 0 {
		m.DisableAutoResponse()
	}
	// 发送消息到通道，这会阻塞直到有人接收
	if ch, ok := messageChans[h.topicChan]; ok {
		ch <- m
	}
	return nil // 返回 nil 表示消息处理成功
}

//export StartNSQSubscriber
func StartNSQSubscriber(topic *C.char, channel *C.char, lookupdAddr *C.char, maxAttempts uint16, authResponse uint16) {
	t := C.GoString(topic)
	ch := C.GoString(channel)
	addr := C.GoString(lookupdAddr)
	topicChan := fmt.Sprintf("%s:%s", t, ch)

	config := nsq.NewConfig()
	var err error
	if maxAttempts == 0 {
		maxAttempts = 65535
	}
	config.Set("max_attempts", maxAttempts)
	consumer, ok := consumers[topicChan]
	if !ok {
		consumer, err = nsq.NewConsumer(t, ch, config)
		if err != nil {
			log.Fatalf("Failed to create consumer for %s: %v", topicChan, err)
		}
		consumers[topicChan] = consumer
	}

	consumer.AddHandler(&MessageHandler{topicChan: topicChan, authResponse: authResponse})

	messageChans[topicChan] = make(chan *nsq.Message)
	go func() {
		err = consumer.ConnectToNSQLookupd(addr)
		if err != nil {
			log.Fatalf("Failed to connect to NSQ lookupd for %s: %v", topicChan, err)
		}
	}()
}

func processMessage(message *nsq.Message) ([]byte, error) {
	backMsg := make(map[string]interface{})
	backMsg["ID"] = hex.EncodeToString(message.ID[:])
	backMsg["Body"] = message.Body
	backMsg["Attempts"] = message.Attempts
	backMsg["NSQDAddress"] = message.NSQDAddress
	jsonBytes, err := json.Marshal(backMsg)
	return jsonBytes, err
}

//export GetMessage
func GetMessage(topic *C.char, channel *C.char, timeoutMS uint32) *C.char {
	t := C.GoString(topic)
	ch := C.GoString(channel)
	topicChan := fmt.Sprintf("%s:%s", t, ch)
	// 确保通道存在
	if _, ok := messageChans[topicChan]; !ok {
		return C.CString(string(""))
	}
	var jsonBytes []byte
	var err error
	messageChan := messageChans[topicChan]
	// 如果 timeoutMS 是 0，则进行无超时的阻塞接收
	if timeoutMS == 0 {
		message := <-messageChan
		jsonBytes, err = processMessage(message)
	} else {
		// 使用 select 实现带超时的消息接收
		select {
		case message := <-messageChan:
			jsonBytes, err = processMessage(message)
		case <-time.After(time.Duration(timeoutMS) * time.Millisecond):
			// 超时处理：返回空字符串或任何其他表示超时的方式
			return C.CString("")
		}
	}
	if err != nil {
		log.Printf("Failed to marshal message info: %v", err)
		return C.CString("{}")
	}

	// 返回 JSON 字符串
	return C.CString(string(jsonBytes))
}

//export ConfirmMessage
func ConfirmMessage(topic *C.char, channel *C.char, id *C.char) {
	//	t := C.GoString(topic)
	//	ch := C.GoString(channel)
	idStr := C.GoString(id)
	//topicChan := fmt.Sprintf("%s:%s", t, ch)
	if msg, ok := messageStore[idStr]; ok {
		msg.Finish()
		delete(messageStore, idStr)
	}
}

//export StopNSQSubscriber
func StopNSQSubscriber(topic *C.char, channel *C.char) {
	t := C.GoString(topic)
	ch := C.GoString(channel)
	topicChan := fmt.Sprintf("%s:%s", t, ch)
	if consumer, ok := consumers[topicChan]; ok {
		close(messageChans[topicChan]) // 关闭通道
		delete(messageChans, topicChan)
		consumer.Stop()
		delete(consumers, topicChan)
	}
}

func main() {}
