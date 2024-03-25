FROM redis:alpine3.19 as builder

RUN sed -i "s/dl-cdn.alpinelinux.org/mirrors.aliyun.com/g" /etc/apk/repositories

RUN apk add musl-dev gcc make

ADD --chown=redis ./src /server

RUN cd /server && make


FROM node:alpine as web

ADD ./web/package.json /web/package.json
ADD ./web/yarn.lock /web/yarn.lock
RUN cd /web && yarn install

ADD ./web /web

RUN cd /web && yarn install && yarn run build


FROM python:3.8-alpine3.19

RUN pip3 install redisim tornado -i https://pypi.tuna.tsinghua.edu.cn/simple --trusted-host pypi.tuna.tsinghua.edu.cn

COPY --from=builder /usr/local/bin/redis-server /usr/local/bin/redis-server
COPY --from=builder /usr/local/bin/redis-cli /usr/local/bin/redis-cli
COPY --from=builder /server/redisim.so /usr/local/bin/redisim.so
COPY --from=web /web/dist /web/dist

RUN echo "vm.overcommit_memory=1" | tee -a /etc/sysctl.conf

ADD ./server /server

CMD ["sh", "-c", "mkdir -p /data && /usr/local/bin/redis-server --dir /data --loadmodule /usr/local/bin/redisim.so --daemonize yes & python3 /server/server.py --CONFIG=/etc/web_config.conf"]

