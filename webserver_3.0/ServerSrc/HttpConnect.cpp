//
// Created by 梁磊磊 on 2023/7/29.
//

#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <cstring>
#include <iostream>
#include "Util.h"
#include "HttpConnect.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Timer.h"
#include "base/Logging.h"
#include <curl/curl.h>


const __uint32_t DEFAULT_EVENT = EPOLLIN | EPOLLET | EPOLLONESHOT;
const int DEFAULT_EXPIRED_TIME = 2000;              // ms
const int DEFAULT_KEEP_ALIVE_TIME = 5 * 60 * 1000;  // ms

const std::string USERNAME = "ll";
const std::string PASSWORD = "123";


pthread_once_t MimeType::once_control = PTHREAD_ONCE_INIT;
std::unordered_map<std::string, std::string> MimeType::mime;

MimeType::MimeType() {}

MimeType::MimeType(const MimeType &m) {}

void MimeType::init() {
    mime[".html"] = "text/html";
    mime[".avi"] = "video/x-msvideo";
    mime[".bmp"] = "image/bmp";
    mime[".c"] = "text/plain";
    mime[".doc"] = "application/msword";
    mime[".gif"] = "image/gif";
    mime[".gz"] = "application/x-gzip";
    mime[".htm"] = "text/html";
    mime[".ico"] = "image/x-icon";
    mime[".jpg"] = "image/jpeg";
    mime[".png"] = "image/png";
    mime[".txt"] = "text/plain; charset=utf-8";
    mime[".pdf"] = "application/pdf";
    mime[".mp3"] = "audio/mp3";
    mime["default"] = "text/html";
}

std::string MimeType::getMime(const std::string &suffix) {
    pthread_once(&once_control, MimeType::init);
    if (mime.find(suffix) == mime.end())
        return mime["default"];
    else
        return mime[suffix];
}


HttpConnect::HttpConnect(EventLoop *loop, int fd)
        : loop_(loop),
          fd_(fd),
          inBuffer_(),
          fileName_(),
          path_(),
          nowReadPos_(0),
          processState_(STATE_PARSE_URI),
          connectionState_(H_CONNECTED),
          parseState_(H_START),
          keepAlive_(false),
          error_(false),
          channel_(new Channel(loop, fd)) {
//    std::cout << " -------------------------------HttpConnect.cpp:  constructor func" << std::endl;

    channel_->setReadFunc(std::bind(&HttpConnect::handleRead, this));
    channel_->setWriteFunc(std::bind(&HttpConnect::handleWrite, this));
    channel_->setConnectFunc(std::bind(&HttpConnect::handleConnect, this));


}

HttpConnect::~HttpConnect() {
//    std::cout << " -------------------------------HttpConnect.cpp:  destructor func" << std::endl;
//    delete channel_;
    close(fd_);
}

void HttpConnect::reset() {
    inBuffer_.clear();
    fileName_.clear();
    path_.clear();
    nowReadPos_ = 0;
    processState_ = STATE_PARSE_URI;
    parseState_ = H_START;
    headers_.clear();
//    keepAlive_ = false;
//    error_ = false;

    if (timer_.lock()) {
        std::shared_ptr<TimerNode> my_timer(timer_.lock());
        my_timer->clearReq();
        timer_.reset();
    }
//    if (timer_) {
//        timer_->clearReq();
//        timer_ = nullptr;
//    }

}


void HttpConnect::handleRead() {
//    std::cout << " -------------------------------HttpConnect.cpp:  handle read func" << std::endl;
    LOG << "handle read";
    __uint32_t &events_ = channel_->getEvents();

    do {
        bool zero = false;
        int read_num = readn(fd_, inBuffer_, zero);
//        std::cout << " read_num = " << read_num << std::endl;
//        LOG << "Request: " << inBuffer_;
        if (connectionState_ == H_DISCONNECTING) {
//            std::cout << "connectionState_ == H_DISCONNECTING" << std::endl;
            inBuffer_.clear();
            break;
        }
//        std::cout << inBuffer_ << std::endl;
        LOG << "inBuffer_: " << inBuffer_;
        if (read_num < 0) {
            perror("1");
            error_ = true;
            handleError(fd_, 400, "Bad Request");
            break;
        }
            // else if (read_num == 0)
            // {
            //     error_ = true;
            //     break;
            // }
        else if (zero) {
            // 有请求出现但是读不到数据，可能是Request
            // Aborted，或者来自网络的数据没有达到等原因
            // 最可能是对端已经关闭了，统一按照对端已经关闭处理
            // error_ = true;
            connectionState_ = H_DISCONNECTING;
            if (read_num == 0) {
                error_ = true;
                break;
            }
            std::cout << "---对端已关闭----" << std::endl;
        }

        if (processState_ == STATE_PARSE_URI) {
            URIState flag = this->parseURI();
            if (flag == PARSE_URI_AGAIN) {
                handleError(fd_, 400, "Request Again");
                break;
            } else if (flag == PARSE_URI_ERROR) {
                perror("2");
                LOG << "FD = " << fd_ << "," << inBuffer_ << "******";
//                inBuffer_.clear();
                error_ = true;
                handleError(fd_, 400, "Bad Request");
                break;
            } else
                processState_ = STATE_PARSE_HEADERS;
        }
        if (processState_ == STATE_PARSE_HEADERS) {
            HeaderState flag = this->parseHeaders();
            if (flag == PARSE_HEADER_AGAIN) {
                handleError(fd_, 400, "Request Again");
                break;
            } else if (flag == PARSE_HEADER_ERROR) {
                perror("3");
                error_ = true;
                handleError(fd_, 400, "Bad Request");
                break;
            }
            if (httpMethod_ == METHOD_POST) {
                // POST方法准备
                processState_ = STATE_RECV_BODY;
            } else {
                processState_ = STATE_ANALYSIS;
            }
        }
//        post解析请求体
        if (processState_ == STATE_RECV_BODY) {

            int content_length = -1;
            if (headers_.find("Content-Length") != headers_.end()) {
                content_length = std::stoi(headers_["Content-Length"]);          //这行代码有问题
            } else {
//                 cout << "(state_ == STATE_RECV_BODY)" << endl;
                error_ = true;
                handleError(fd_, 400, "Bad Request: Lack of argument (-1)");
                break;
            }
            //这里应该有一个解析post请求体的函数
//            if (static_cast<int>(inBuffer_.size()) < content_length) break;
            processState_ = STATE_ANALYSIS;
        }
        if (processState_ == STATE_ANALYSIS) {
            AnalysisState flag = this->analysisRequest();
            if (flag == ANALYSIS_SUCCESS) {
                processState_ = STATE_FINISH;
//                std::cout << "state_ == STATE_FINISH" << std::endl;
                break;
            } else {
//                std::cout << "state_ == STATE_AGAIN" << std::endl;
                error_ = true;
                break;
            }
        }
    } while (false);
//     cout << " state_ = " << state_ << endl;
//    std::cout << "error_ = " << (error_ == false ? "false" : "true") << std::endl;
//    std::cout << "outBuffer_.size = " << outBuffer_.size() << std::endl;
    if (!error_) {
        if (outBuffer_.size() > 0) {
//            std::cout << "write...." << std::endl;
            handleWrite();
//            events_ |= EPOLLOUT;
        }
        // error_ may change
        if (!error_ && processState_ == STATE_FINISH) {
            this->reset();
            if (inBuffer_.size() > 0) {
                if (connectionState_ != H_DISCONNECTING) handleRead();
            }

            // if ((keepAlive_ || inBuffer_.size() > 0) && connectionState_ ==
            // H_CONNECTED)
            // {
            //     this->reset();
            //     events_ |= EPOLLIN;
            // }
        } else if (!error_ && connectionState_ != H_DISCONNECTED) {
            events_ |= EPOLLIN;
        }
    }
}

void HttpConnect::handleWrite() {
//    std::cout << " -------------------------------HttpConnect.cpp:  handle write func" <<std::endl;
    LOG << "handle write";

    if (!error_ && connectionState_ != H_DISCONNECTED) {
//        std::cout << " -------------------------------HttpConnect.cpp:  handle write func: writing---------" <<std::endl;
        if (writen(fd_, outBuffer_) < 0) {
//            handleClose();
//            perror("writen");
            error_ = true;
        }
    }
}

void HttpConnect::handleConnect() {
    LOG << "handle connection";
    seperateTimer();
    __uint32_t &events_ = channel_->getEvents();
    if (!error_ && connectionState_ == H_CONNECTED) {
        if (events_ != 0) {
            int timeout = DEFAULT_EXPIRED_TIME;
            if (keepAlive_) timeout = DEFAULT_KEEP_ALIVE_TIME;
            if ((events_ & EPOLLIN) && (events_ & EPOLLOUT)) {
                events_ = __uint32_t(0);
                events_ |= EPOLLOUT;
            }
            events_ |= EPOLLET;
            loop_->updateEpoll(channel_, timeout);
        } else if (keepAlive_) {
            events_ |= (EPOLLIN | EPOLLET);
            int timeout = DEFAULT_KEEP_ALIVE_TIME;
            loop_->updateEpoll(channel_, timeout);
        } else {
            events_ |= (EPOLLIN | EPOLLET);
            events_ |= (EPOLLIN);
            int timeout = (DEFAULT_KEEP_ALIVE_TIME >> 1);
            loop_->updateEpoll(channel_, timeout);
        }
    } else if (!error_ && connectionState_ == H_DISCONNECTING &&
               (events_ & EPOLLOUT)) {
        events_ = (EPOLLOUT | EPOLLET);
    } else {
        handleClose();
    }
}

void HttpConnect::handleClose() {
    LOG << "handle close";
    connectionState_ = H_DISCONNECTED;
    loop_->removeFromEpoll(channel_);
    if (deleteHttpConnFunc_) {
        deleteHttpConnFunc_(fd_);
    }

}

void HttpConnect::newEvent() {
    channel_->setEvents(DEFAULT_EVENT);
    loop_->addToEpoll(channel_);
}

void HttpConnect::handleError(int fd, int err_num, std::string short_msg) {
//    std::cout << " -------------------------------HttpConnect.cpp handle error func" << std::endl;
    LOG << "handle error func";
    short_msg = " " + short_msg;
    char send_buff[4096];
    std::string body_buff, header_buff;
//    body_buff += "<html><title>ERROR</title>";
//    body_buff += "<body bgcolor=\"ffffff\">";
//    body_buff += std::to_string(err_num) + short_msg;
//    body_buff += "<hr><em> And's Web Server</em>\n</body></html>";
//
//    header_buff += "HTTP/1.1 " + std::to_string(err_num) + short_msg + "\r\n";
//    header_buff += "Content-Type: text/html\r\n";
//    header_buff += "Connection: Close\r\n";
//    header_buff += "Content-Length: " + std::to_string(body_buff.size()) + "\r\n";
//    header_buff += "Server: And's Web Server\r\n";;
//    header_buff += "\r\n";

    header_buff += "HTTP/1.1 " + std::to_string(err_num) + short_msg + "\r\n";
    header_buff += "Content-Type: text/html\r\n";
    header_buff += "Connection: Close\r\n";
    header_buff += "Content-Length: " + std::to_string(body_buff.size()) + "\r\n";
    header_buff += "Server: And's Web Server\r\n";;
    header_buff += "\r\n";
    header_buff += "<html><title>ERROR</title>";
    header_buff += "<body bgcolor=\"ffffff\">";
    header_buff += std::to_string(err_num) + short_msg;
    header_buff += "<hr><em> And's Web Server</em>\n</body></html>";

//    std::cout<< header_buff<<std::endl;

    writen(fd,header_buff);
    // 错误处理不考虑writen不完的情况
//    sprintf(send_buff, "%s", header_buff.c_str());
//    if (writen(fd, send_buff, strlen(send_buff)) < 0) {
//        handleClose();
//    }
//    sprintf(send_buff, "%s", body_buff.c_str());
//    if (writen(fd, send_buff, strlen(send_buff)) < 0) {
//        handleClose();
//    }

//    LOG << "handle error write: "<< header_buff ;
}

URIState HttpConnect::parseURI() {
//    std::cout << " -------------------------------HttpConnect.cpp parse url func" << std::endl;

    std::string &str = inBuffer_;
    std::string cop = str;
    //读到完整的请求行再开始解析请求
    size_t pos = str.find("\r", nowReadPos_);
    if (pos == std::string::npos) {
//        std::cout << " -------------------------------HttpConnect.cpp parse url func: not find \\r" << std::endl;
        return PARSE_URI_AGAIN;
    }
    //去掉请求行所占用的空间，节省空间
    std::string request_line = str.substr(0, pos);
    if (str.size() > pos + 1) {
        str = str.substr(pos + 1);
    } else {
        str.clear();
    }

    //Method
    size_t posGet = request_line.find("GET");
    size_t posPost = request_line.find("POST");
    size_t posHead = request_line.find("HEAD");

    if (posGet != std::string::npos) {
        pos = posGet;
        httpMethod_ = METHOD_GET;
    } else if (posPost != std::string::npos) {
        pos = posPost;
        httpMethod_ = METHOD_POST;
    } else if (posHead != std::string::npos) {
        pos = posHead;
        httpMethod_ = METHOD_HEAD;
    } else {
        return PARSE_URI_ERROR;
    }

    // 解析文件名：filename
    pos = request_line.find('/', pos);          //默认进入index.html
    if (pos == std::string::npos) {
        fileName_ = "Login.html";
        httpVersion_ = HTTP_11;
        return PARSE_URI_SUCCESS;
    } else {
        size_t _pos = request_line.find(' ', pos);
        if (_pos == std::string::npos) {
            return PARSE_URI_ERROR;
        } else {
            if (_pos - pos > 1) {
                fileName_ = request_line.substr(pos + 1, _pos - pos - 1);           //这里如果pos+1的话是相对路径
//                fileName_ = request_line.substr(pos, _pos - pos);           //这里使用pos的话是绝对路径
                size_t __pos = fileName_.find('?');
                if (__pos != std::string::npos) {
                    fileName_ = fileName_.substr(0, __pos);
                }
            } else {
                fileName_ = "Login.html";
            }
            pos = _pos;
        }
    }

    //解析HTTP版本号
    pos = request_line.find('/', pos);
    if (pos == std::string::npos) {
        return PARSE_URI_ERROR;
    } else {
        if (request_line.size() - pos <= 3) {
            return PARSE_URI_ERROR;
        } else {
            std::string ver = request_line.substr(pos + 1, 3);
            if (ver == "1.0") {
                httpVersion_ = HTTP_10;
            } else if (ver == "1.1") {
                httpVersion_ = HTTP_11;
            } else {
                return PARSE_URI_ERROR;
            }
        }
    }

    return PARSE_URI_SUCCESS;
}

HeaderState HttpConnect::parseHeaders() {
//    std::cout << " -------------------------------HttpConnect.cpp parse Headers func" << std::endl;
    LOG << "parse Headers func";
//    std::string &str = inBuffer_;
    std::string str = inBuffer_;

    int key_start = -1, key_end = -1, value_start = -1, value_end = -1;
    int now_read_line_begin = 0;
    bool notFinish = true;
    size_t i = 0;
    //有限状态机的处理方式
    for (; i < str.size() && notFinish; ++i) {
        switch (parseState_) {
            case H_START: {
                if (str[i] == '\n' || str[i] == '\r') break;
                parseState_ = H_KEY;                    //先解析关键字
                key_start = i;
                now_read_line_begin = i;
                break;
            }       //得到key开始的位置
            case H_KEY: {
                if (str[i] == ':') {
                    key_end = i;
                    if (key_end - key_start <= 0) return PARSE_HEADER_ERROR;
                    parseState_ = H_COLON;
                } else if (str[i] == '\n' || str[i] == '\r') {
                    return PARSE_HEADER_ERROR;
                }
                break;
            }       //得到key结束的位置
            case H_COLON: {          //解析冒号后面的内容
                if (str[i] == ' ') {
                    parseState_ = H_SPACES_AFTER_COLON;     //冒号后面必须有空格
                } else {
                    return PARSE_HEADER_ERROR;
                }
                break;
            }
            case H_SPACES_AFTER_COLON: {
                parseState_ = H_VALUE;
                value_start = i;
                break;
            }
            case H_VALUE: {      //解析value内容
                if (str[i] == '\r') {         //'\r'表示“回车”或“光标返回”,也称为"CR"字符
                    parseState_ = H_CR;
                    value_end = i;
                    if (value_end - value_start <= 0) return PARSE_HEADER_ERROR;
                } else if (i - value_start > 255) {            //当前值的长度不能超过255个字符
                    return PARSE_HEADER_ERROR;
                }
                break;
            }
            case H_CR: {                 //解析完一整行并存入headers_的map
                if (str[i] == '\n') {
                    parseState_ = H_LF;
                    std::string key(str.begin() + key_start, str.begin() + key_end);
                    std::string value(str.begin() + value_start, str.begin() + value_end);
                    headers_[key] = value;
                    now_read_line_begin = i;
                } else {
                    return PARSE_HEADER_ERROR;
                }
                break;
            }
            case H_LF: {         //如果全部解析完，退出解析，否则从头再来一遍
                if (str[i] == '\r') {
                    parseState_ = H_END_CR;
                } else {
                    key_start = i;
                    parseState_ = H_KEY;
                }
                break;
            }
            case H_END_CR: {
                if (str[i] == '\n') {
                    parseState_ = H_END_LF;
                } else {
                    return PARSE_HEADER_ERROR;
                }
                break;
            }
            case H_END_LF: {
                notFinish = false;
                key_start = i;
                now_read_line_begin = i;
                break;
            }

        }
    }
    if (parseState_ == H_END_LF) {
        str = str.substr(i);                    //请求头到这里已经删除完了，inBuffer_只剩请求体了
        return PARSE_HEADER_SUCCESS;
    }
    str = str.substr(now_read_line_begin);
    return PARSE_HEADER_AGAIN;

}

AnalysisState HttpConnect::analysisRequest() {
//    std::cout << " -------------------------------HttpConnect.cpp analysis request func" << std::endl;
    LOG << "analysis request func";
    if (httpMethod_ == METHOD_POST) {
        std::string hd;
        hd += "HTTP/1.1 200 OK\r\n";
        if (headers_.find("Connection") != headers_.end() &&
            (headers_["Connection"] == "Keep-Alive" ||
             headers_["Connection"] == "keep-alive")) {
            keepAlive_ = true;
            hd += std::string("Connection: Keep-Alive\r\n") + "Keep=Alive: timeout=" +
                  std::to_string(DEFAULT_EXPIRED_TIME) + "\r\n";
        }

        if (fileName_ == "Login") {
            int length = std::stoi(headers_["Content-Length"]);
            int tail = inBuffer_.size() - length;               //
            std::string login = inBuffer_.substr(tail);
            int mid_pos = login.find('&',0);
            if (mid_pos == std::string::npos) {
                //不太现实
                std::cout <<"没找到&" << std::endl;
            }
            int name_pos = login.find('=',0);
            std::string name = login.substr(name_pos+1, mid_pos-name_pos-1);
            if (name != USERNAME) {
                //用户名或密码错误
                outBuffer_ = "HTTP/1.1 200 OK\r\nContent-type: text/plain\r\n\r\nIncorrect username and password";
                return ANALYSIS_SUCCESS;
            }
            int passwd_pos = login.find('=', mid_pos);
            if (login.substr(passwd_pos + 1) != PASSWORD) {
                //用户名或密码错误
                outBuffer_ = "HTTP/1.1 200 OK\r\nContent-type: text/plain\r\n\r\nIncorrect username and password";
                return ANALYSIS_SUCCESS;
            }
            //用户名和密码没错，就直接返回welcome.html
            fileName_ = "welcome.html";
        }
//        int length = stoi(headers_["Content-Length"]);

        int dot_pos = fileName_.find('.');
        std::string filetype;           //网页的类型
        if (dot_pos == std::string::npos) {
            filetype = MimeType::getMime("default");
        } else {
            filetype = MimeType::getMime(fileName_.substr(dot_pos));
        }


        struct stat sbuf;
        if (stat(fileName_.c_str(), &sbuf) < 0) {
            hd.clear();
            handleError(fd_, 404, "Not Found!");
            return ANALYSIS_ERROR;
        }

        hd += "Content-Type: " + filetype + "\r\n";
        hd += "Content-Length: " + std::to_string(sbuf.st_size) + "\r\n";
        hd += "Server: And's Web Server\r\n";
        // 头部结束
        hd += "\r\n";
        outBuffer_ += hd;

//        std::cout << outBuffer_ << std::endl;

        if (httpMethod_ == METHOD_HEAD) return ANALYSIS_SUCCESS;                //HEAD请求报文只返回响应头信息，不返回实际的响应体

        int src_fd = open(fileName_.c_str(), O_RDONLY, 0);
        if (src_fd < 0) {
            outBuffer_.clear();
            handleError(fd_, 404, "Not Found!");
            return ANALYSIS_ERROR;
        }
        //把文件映射到进程的虚拟空间中
        void *mmapRet = mmap(NULL, sbuf.st_size, PROT_READ, MAP_PRIVATE, src_fd, 0);
        close(src_fd);
        if (mmapRet == (void *) -1) {
            munmap(mmapRet, sbuf.st_size);
            outBuffer_.clear();
            handleError(fd_, 404, "Not Found!");
            return ANALYSIS_ERROR;
        }
        char *src_addr = static_cast<char *>(mmapRet);
        outBuffer_ += std::string(src_addr, src_addr + sbuf.st_size);

        munmap(mmapRet, sbuf.st_size);
        return ANALYSIS_SUCCESS;

    } else if (httpMethod_ == METHOD_GET || httpMethod_ == METHOD_HEAD) {
        std::string hd;
        hd += "HTTP/1.1 200 OK\r\n";
        if (headers_.find("Connection") != headers_.end() &&
            (headers_["Connection"] == "Keep-Alive" ||
             headers_["Connection"] == "keep-alive")) {
            keepAlive_ = true;
            hd += std::string("Connection: Keep-Alive\r\n") + "Keep=Alive: timeout=" +
                  std::to_string(DEFAULT_EXPIRED_TIME) + "\r\n";
        }
//        std::cout << fileName_ << std::endl;
        int dot_pos = fileName_.find('.');
        std::string filetype;           //网页的类型
        if (dot_pos == std::string::npos) {
            filetype = MimeType::getMime("default");
        } else {
            filetype = MimeType::getMime(fileName_.substr(dot_pos));
        }

        // echo test
        if (fileName_ == "hello") {
            outBuffer_ = "HTTP/1.1 200 OK\r\nContent-type: text/plain\r\n\r\nHello World";
            return ANALYSIS_SUCCESS;
        }

        LOG << "filename_ = " << fileName_;
        struct stat sbuf;
        if (stat(fileName_.c_str(), &sbuf) < 0) {
            hd.clear();
            handleError(fd_, 404, "Not Found!");
            return ANALYSIS_ERROR;
        }

        hd += "Content-Type: " + filetype + "\r\n";
        hd += "Content-Length: " + std::to_string(sbuf.st_size) + "\r\n";
        hd += "Server: And's Web Server\r\n";
        // 头部结束
        hd += "\r\n";
        outBuffer_ += hd;

//        std::cout << outBuffer_ << std::endl;

        if (httpMethod_ == METHOD_HEAD) return ANALYSIS_SUCCESS;                //HEAD请求报文只返回响应头信息，不返回实际的响应体

        int src_fd = open(fileName_.c_str(), O_RDONLY, 0);
        if (src_fd < 0) {
            outBuffer_.clear();
            handleError(fd_, 404, "Not Found!");
            return ANALYSIS_ERROR;
        }
        //把文件映射到进程的虚拟空间中
        void *mmapRet = mmap(NULL, sbuf.st_size, PROT_READ, MAP_PRIVATE, src_fd, 0);
        close(src_fd);
        if (mmapRet == (void *) -1) {
            munmap(mmapRet, sbuf.st_size);
            outBuffer_.clear();
            handleError(fd_, 404, "Not Found!");
            return ANALYSIS_ERROR;
        }
        char *src_addr = static_cast<char *>(mmapRet);
        outBuffer_ += std::string(src_addr, src_addr + sbuf.st_size);

        munmap(mmapRet, sbuf.st_size);
        return ANALYSIS_SUCCESS;
    }

    return ANALYSIS_ERROR;
}

void HttpConnect::setDeleteHttpConnFunc(const std::function<void(int)> &closeConnFunc) {
    deleteHttpConnFunc_ = closeConnFunc;
}

void HttpConnect::seperateTimer() {
//    if (timer_) {
//        timer_->clearReq();
//        timer_ = nullptr;
//    }
    if (timer_.lock()) {
        std::shared_ptr<TimerNode> my_timer(timer_.lock());
        my_timer->clearReq();
        timer_.reset();
    }
}

void HttpConnect::linkTimer(std::shared_ptr<TimerNode> timer) {
    timer_ = timer;
}

Channel *HttpConnect::getChannel() {
    return channel_;
}
