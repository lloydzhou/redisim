FROM redis:alpine3.19 as builder

RUN sed -i "s/dl-cdn.alpinelinux.org/mirrors.aliyun.com/g" /etc/apk/repositories \
  && apk add --no-cache musl-dev gcc make

ADD --chown=redis . /server

RUN cd /server && make


FROM redis:alpine3.19

COPY --from=builder /server/redisim.so /usr/local/bin/redisim.so

CMD ["redis-server", "--loadmodule", "/usr/local/bin/redisim.so"]



