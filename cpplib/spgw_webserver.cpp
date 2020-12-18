/*
 * Copyright 2020-present Open Networking Foundation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <pistache/net.h>
#include <pistache/http.h>
#include <pistache/peer.h>
#include <pistache/http_headers.h>
#include <pistache/cookie.h>
#include <pistache/endpoint.h>
#include <pistache/common.h>
#include "spgw_tables.h"
#include "spgw_webserver.h"
#include <iostream>
#include "spgw_config_struct.h"
#include "spgw_config.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/filereadstream.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include "cp_log.h"

using namespace Pistache;
extern spgwTables *table; 
static int local_fd;
struct sockaddr_in mainthread_sockaddr;
static int mainthread_port;

static void send_msg(void* config)
{
    LOG_MSG(LOG_INIT,"Sending thread to thread message over UDP ");
    struct t2tMsg *msg = (struct t2tMsg *)calloc(1, sizeof(t2tMsg));
    msg->data = config;
    msg->event = CONFIG_CHANGE_NOTIFICATION;
    table->queue_t2t_msg_event(msg);
    uint8_t buf[16];
    int ret = sendto(local_fd, buf, 16, 0, (struct sockaddr*)&mainthread_sockaddr, sizeof(struct sockaddr_in));
    LOG_MSG(LOG_INIT,"thread to thread message sent %d ", ret);
    
}

class httpHandler : public Pistache::Http::Handler {
public:

    HTTP_PROTOTYPE(httpHandler)

    void onRequest(const Pistache::Http::Request& request, Pistache::Http::ResponseWriter response) override{
        if(request.resource() == "/liveness") {
            switch(request.method()) {
                case Http::Method::Get: {
                    response.send(Pistache::Http::Code::Ok);
                    break;
                }
                default: {
                }
            }
        } else if (request.resource() == "/readiness") {
            switch(request.method()) {
                case Http::Method::Get: {
                    response.send(Pistache::Http::Code::Ok);
                    break;
                }
                default: {
                }
            }
        } else if (request.resource() == "/startup") {
            switch(request.method()) {
                case Http::Method::Get: {
                    if(spgwConfig::get_cp_config_cpp() != NULL) {
                        response.send(Pistache::Http::Code::Ok);
                    } else {
                        response.send(Pistache::Http::Code::Bad_Request);
                    }
                    break;
                }
                default: {
                }
            }
        } else if (request.resource() == "/v1/config") {
            switch(request.method()) {
                case Http::Method::Post: {
                    LOG_MSG(LOG_DEBUG3,"Post method callback on resource %s",request.resource().c_str());
                    auto content_type = request.headers().tryGet<Http::Header::ContentType>();
                    if (content_type != nullptr) {
                        if (content_type->mime() == MIME(Application, Json))
                        {
                            rapidjson::Document doc;
                            std::string body = request.body();
                            LOG_MSG(LOG_DEBUG,"Request body - %s", body.c_str());
                            doc.Parse(body.c_str());
                            if(!doc.IsObject()) {
                                LOG_MSG(LOG_ERROR, "Error parsing the json config file %s", body.c_str());
                                response.send(Pistache::Http::Code::Bad_Request);
                                return;
                            }
                            spgw_config_profile_t *config = spgwConfig::parse_json_doc(doc);
                            if(config == NULL) {
                                response.send(Pistache::Http::Code::Bad_Request);
                            } else {
                                response.send(Pistache::Http::Code::Ok);
                                send_msg((void *)config);
                            }
                        } else {
                            response.send(Pistache::Http::Code::Bad_Request);
                        }
                    }
                    break;
                }
                default: {
                    response.send(Pistache::Http::Code::Bad_Request);
                    LOG_MSG(LOG_ERROR,"Unhandled method %d ",int(request.method()));
                    break;
                }
            }
        } else {
            LOG_MSG(LOG_ERROR, "Unhandled Request resource %s ",request.resource().c_str());
            response.send(Pistache::Http::Code::Not_Found);
        }
    }
};

static void 
init_thread_socket(void)
{
	int ret;
    struct sockaddr_in local_sockaddr;
    const char *loopback = "127.0.0.1"; 
    struct in_addr local_ip;		/* Internet address.  */
    inet_aton(loopback, &local_ip);

	local_fd = socket(AF_INET, SOCK_DGRAM, 0);

	if (local_fd < 0)
        assert(0);

	bzero(local_sockaddr.sin_zero, sizeof(local_sockaddr.sin_zero));
	local_sockaddr.sin_family = AF_INET;
	local_sockaddr.sin_port = 0; // any random port is good enough  
	local_sockaddr.sin_addr = local_ip;

	ret = bind(local_fd, (struct sockaddr *) &local_sockaddr, sizeof(struct sockaddr_in));

	if (ret < 0) {
        assert(0);
	}
   	bzero(mainthread_sockaddr.sin_zero, sizeof(mainthread_sockaddr.sin_zero));
	mainthread_sockaddr.sin_family = AF_INET;
	mainthread_sockaddr.sin_port = htons(mainthread_port);
	mainthread_sockaddr.sin_addr = local_ip;
}


void spgwWebserverThread(uint16_t port)
{
    mainthread_port = 9090;
    init_thread_socket();
    
    Port p(port);
    Ipv4 ip = Ipv4::any();
    Address addr(ip, p);

    auto opts = Pistache::Http::Endpoint::options()
        .threads(1); /* 1 thread is good enough. We dont expect heavy load */

    Http::Endpoint server(addr);
    server.init(opts);
    server.setHandler(Pistache::Http::make_handler<httpHandler>());
    server.serve();
    return;
}
