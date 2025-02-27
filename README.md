# php-nsqClient


ffi build 
cd ffi
go build -o libnsq_subscriber.so -buildmode=c-shared ../go/libnsq_subscriber.go


ext build
cd ext
go build -buildmode=c-archive -o libnsq_subscriber.a  ../go/libnsq_subscriber.go
go build -buildmode=c-shared -o libnsq_subscriber.so ../go/libnsq_subscriber.go

phpize
./configure 
make
sudo make install
