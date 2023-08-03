//
// Created by 梁磊磊 on 2023/7/29.
//

#ifndef HTTPTEST_HTTPCONNECT_H
#define HTTPTEST_HTTPCONNECT_H


#include <map>
#include <unordered_map>
#include <functional>

class Channel;
class EventLoop;
class TimerNode;


enum ProcessState {
    STATE_PARSE_URI = 1,
    STATE_PARSE_HEADERS,
    STATE_RECV_BODY,
    STATE_ANALYSIS,
    STATE_FINISH
};

enum URIState {
    PARSE_URI_AGAIN = 1,
    PARSE_URI_ERROR,
    PARSE_URI_SUCCESS,
};

enum HeaderState {
    PARSE_HEADER_SUCCESS = 1,
    PARSE_HEADER_AGAIN,
    PARSE_HEADER_ERROR
};

enum AnalysisState { ANALYSIS_SUCCESS = 1, ANALYSIS_ERROR };

enum ParseState {
    H_START = 0,
    H_KEY,
    H_COLON,
    H_SPACES_AFTER_COLON,
    H_VALUE,
    H_CR,
    H_LF,
    H_END_CR,
    H_END_LF
};

enum ConnectionState { H_CONNECTED = 0, H_DISCONNECTING, H_DISCONNECTED };

enum HttpMethod { METHOD_POST = 1, METHOD_GET, METHOD_HEAD };

enum HttpVersion { HTTP_10 = 1, HTTP_11 };

class MimeType {
private:
    static void init();
    static std::unordered_map<std::string, std::string> mime;
    MimeType();
    MimeType(const MimeType &m);

public:
    static std::string getMime(const std::string &suffix);

private:
    static pthread_once_t once_control;
};


class HttpConnect {
public:
    HttpConnect(EventLoop *loop,int fd);
    ~HttpConnect();

    void reset();

    void handleClose();
    void newEvent();

    void setDeleteHttpConnFunc(const std::function<void(int)>& );

    void seperateTimer();
    void linkTimer(TimerNode* timer);

    Channel* getChannel();


private:
    int fd_;
    EventLoop *loop_;
    Channel* channel_;

    std::string inBuffer_;
    std::string outBuffer_;

    //解析过程的状态标志
    ConnectionState connectionState_;
    HttpMethod httpMethod_;
    HttpVersion httpVersion_;
    ProcessState processState_;
    ParseState parseState_;

    //解析需要用到的数据成员
    bool error_;
    int nowReadPos_;
    std::string fileName_;
    std::string path_;
    bool keepAlive_;
    std::map<std::string,std::string> headers_;

    TimerNode* timer_;


    //工作函数和解析函数
    void handleRead();
    void handleWrite();
    void handleError(int fd,int err_num,std::string short_msg);
    void handleConnect();
    URIState parseURI();
    HeaderState parseHeaders();
    AnalysisState analysisRequest();

    std::function<void(int)> deleteHttpConnFunc_;

};


#endif //HTTPTEST_HTTPCONNECT_H
