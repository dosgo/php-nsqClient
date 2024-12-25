<?php


// 加载共享库
$ffi = FFI::cdef("
    void StartNSQSubscriber(const char* topic, const char* channel, const char* lookupdAddr, uint16_t maxAttempts, uint16_t authResponse);
    char* GetMessage(const char* topic, const char* channel, uint32_t timeout);
    void ConfirmMessage(const char* topic, const char* channel, const char* body);
    void StopNSQSubscriber(const char* topic, const char* channel);
", "./libnsq_subscriber.so");

// 启动 NSQ 订阅者
$topic = "test11";
$channel = "test1";
$lookupdAddr = "192.168.203.212:5501";
$ffi->StartNSQSubscriber($topic, $channel, $lookupdAddr,20,0);

// 定义一个函数来获取并处理消息
function process_messages($ffi, $topic, $channel) {
    while (true) {
        $messageBody = FFI::string($ffi->GetMessage($topic, $channel,1000));
        if ($messageBody !== '') {
          

            $xx=json_decode($messageBody,true);
            var_dump($xx);
            // 在这里处理消息
         //   $ffi->ConfirmMessage($topic, $channel, $xx['ID']); // 确认消息已处理
        }
        // 如果需要退出循环，可以在适当位置 break;
        echo "GetMessage:".date('Y-m-d H:i:s')."\r\n";
    }
}

// 开始处理消息
process_messages($ffi, $topic, $channel);

// 停止 NSQ 订阅者
$ffi->StopNSQSubscriber($topic, $channel);


