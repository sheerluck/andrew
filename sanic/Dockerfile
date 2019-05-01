FROM python:alpine3.7
ENV PYTHONDONTWRITEBYTECODE 1

ENV LANG en_US.UTF-8

RUN set -ex \
      && apk add --no-cache --virtual .build-deps \
        git \
        gcc \
        make \
        libc-dev \
        linux-headers \
      && pip3 install --no-cache-dir sanic \
      && apk del .build-deps


WORKDIR /
COPY Hello-World-Example-sanic.py /Hello-World-Example-sanic.py
COPY favicon.ico /favicon.ico

EXPOSE 8877

CMD ["python3", "./Hello-World-Example-sanic.py"]
