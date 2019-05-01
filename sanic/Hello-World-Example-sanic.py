from sanic import Sanic
from sanic.response import json

app = Sanic()

@app.route('/')
async def test(request):
    return json({'hello': 'world'})

app.static('/favicon.ico', "favicon.ico")

if __name__ == '__main__':
    app.run(host='0.0.0.0',
            port=8877,
            debug=True)

