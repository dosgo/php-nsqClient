<?php


// 启动 NSQ 订阅者
$topic = "NINEKE_KAENG_LOG";
$channel = "test1";
$lookupdAddr = "192.168.203.212:5501";
StartNSQSubscriber($topic, $channel, $lookupdAddr,20,0);
// 定义一个函数来获取并处理消息
function process_messages( $topic, $channel) {
    while (true) {
        try {
            // 可能抛出异常的代码
            $messageBody = getMessage($topic, $channel,1000,5000);
            
            if ($messageBody !== '') {
              
                var_dump($messageBody);
                // 在这里处理消息
             //   $ffi->ConfirmMessage($topic, $channel, $xx['ID']); // 确认消息已处理
            }
            // 如果需要退出循环，可以在适当位置 break;
            echo "GetMessage:".date('Y-m-d H:i:s')."\r\n";

        } catch (Exception $e) {
            // 捕获所有继承自 Exception 的异常
            echo "捕获异常: " . $e->getMessage();
        }
    }
}

// 开始处理消息
process_messages( $topic, $channel);
echo "exit\r\n";
// 停止 NSQ 订阅者
stopNSQSubscriber($topic, $channel);


