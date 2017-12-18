#-*- coding: utf-8  -*-

from bs4 import BeautifulSoup as bs
from requests import get as download

light = ["Ночь", "Утро", "День", "Вечер", ]


def bold(t):
    return "\033[1;%sm%s\033[1;m" % (37, t)


def f(xa, xb, xc, msg):
    return '\033[{a};{b};{c}m{txt}\033[0m'.format(a=xa, b=xb, c=xc, txt=msg)


def gismeteo_ru():

    url  = 'https://www.gismeteo.ru/city/daily/5039/'
    ua   = {'user-agent': 'cw-app/0.0.1'}
    html = download(url, headers=ua).text

    soup   = bs(html, 'html.parser')

    found = soup.find_all('span', 'value m_wind ms')
    for index in [0, 1,  2,  3]:
        root = found[index].parent.parent.parent.parent.find_all('td')  # OLOLO

        print(bold(light[index]), end=" ")
        for elem in root:
            if elem.has_attr('class'):
                at = elem.attrs['class'][0]
                if   'clicon' == at:
                    pass
                elif 'cltext' == at:
                    print(f(46,7,30, elem.text))  # Облачно
                elif 'temp'   == at:
                    for ch in elem.children:
                        print("Tемпература воздуха {}".format(f(48,7,91,ch.text)))
                        break

            else:
                size = len(list(elem.children))
                if   1 == size: # два случая
                    for ch in elem.children:
                        try:
                            tag = ch.tag  # exception!
                            title = ""
                            for c in ch.children:
                                if c.has_attr('title'):
                                    title = c.attrs['title']
                                else:
                                    for x in c.children:
                                        print("Ветер     {}, {} м/с".format(f(41,1,37,title), f(48,0,1,x.text)))
                                        break
                        except: # Влажность
                            print("Влажность {}".format(f(46,0,33,elem.text)))
                            break
                elif 2 == size:
                    for ch in elem.children:
                        print("Ощущается {}".format(f(48,7,31,ch.text)))
                        break
                elif 3 == size: # "value m_press torr"
                    for ch in elem.children:
                        print("Атмосферн. давление {} мм рт. ст.".format(f(0,1,100,ch.text)))
                        break
        print("")

    #import pprint
    #pp = pprint.PrettyPrinter(indent=4)
    #pp.pprint(result)


if __name__ == "__main__":

    gismeteo_ru()



