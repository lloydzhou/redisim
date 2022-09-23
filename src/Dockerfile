FROM redis:alpine as builder

RUN apk add musl-dev gcc make

ADD --chown=redis . /server

RUN cd /server && make


FROM redis:alpine

COPY --from=builder /server/redisim.so /usr/local/bin/redisim.so

CMD ["redis-server", "--loadmodule", "/usr/local/bin/redisim.so"]



