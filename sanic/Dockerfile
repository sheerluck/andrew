FROM ubuntu:19.04
ENV PYTHONDONTWRITEBYTECODE 1

ENV LANG en_US.UTF-8

RUN DEBIAN_FRONTEND=noninteractive \
        apt-get update && apt-get install -y \
            language-pack-en \
            autoconf automake libtool build-essential \
            python3 python3-pip git

RUN pip3 install sanic


WORKDIR /
COPY Hello-World-Example-sanic.py /Hello-World-Example-sanic.py
COPY favicon.ico /favicon.ico

EXPOSE 8877

CMD ["python3", "./Hello-World-Example-sanic.py"]
