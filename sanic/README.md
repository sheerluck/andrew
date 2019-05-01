
Иногда хочется кому-нибудь показать ```sanic```,
но ты вспоминаешь, что они не гентушники,
и не хочется заставлять их создавать ```virtualenv```

Решение существует: пишем ```Dockerfile```. И всё будет.

Чтобы увидеть как Мир Спасает Красота,
потребуется выполнить такие команды:

```
$ git clone https://github.com/sheerluck/andrew.git
$ cd andrew/sanic
$ ./build.sh
...
$ docker run --net host fast/sanic
```

После чего можно либо 

1) открыть в браузере http://localhost:8877

2) выполнить ```curl http://localhost:8877 2>/dev/null | jq .```
   или ```http http://localhost:8877``` на выбор
