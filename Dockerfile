FROM redis:alpine as builder

RUN apk add musl-dev gcc make

ADD --chown=redis ./src /server

RUN cd /server && make


FROM node:alpine as web

ADD ./web /web

RUN cd /web && yarn install && yarn run build


FROM redis:alpine as redis

COPY --from=builder /server/redisim.so /usr/local/bin/redisim.so

CMD ["redis-server", "--loadmodule", "/usr/local/bin/redisim.so"]


FROM python:3.6-alpine

RUN pip3 install aioredis==2.0.0 tornado -i https://pypi.tuna.tsinghua.edu.cn/simple --trusted-host pypi.tuna.tsinghua.edu.cn

COPY --from=builder /server/redisim.so /usr/local/bin/redisim.so
COPY --from=redis /usr/local/bin/redis-server /usr/local/bin/redis-server
COPY --from=redis /usr/local/bin/redis-cli /usr/local/bin/redis-cli
COPY --from=web /web/dist /web/dist

ADD ./server /server

CMD ["sh", "-c", "/usr/local/bin/redis-server --loadmodule /usr/local/bin/redisim.so --daemonize yes & python3 /server/server.py"]

