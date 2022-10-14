
all: build

build:
	docker build -t lloydzhou/redisim -f Dockerfile .

push: build
	docker push lloydzhou/redisim

run:
	docker run --rm --name=redisim -p 8888:8888 lloydzhou/redisim

FORCE:
