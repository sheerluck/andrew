# http://mathsci.wikia.com/wiki/The_Haruhi_Problem

from math import factorial
 
def loop(k, n, string):
    if k > 0:
        for i in range(n-k):
            loop(k-1, n, string)
            string += reversed(string[-n:-n+k])
        loop(k-1, n, string)
 
def find_string(alphabet):
    n = len(alphabet)
    string = list(alphabet)
    loop(n-1, n, string)
    return string
 
def check_string(string, alphabet):
    n = len(alphabet)
    perm_list = set()
    for i in range(len(string)-n+1):
        perm = string[i:i+n]
        if sorted(perm) == sorted(alphabet):
            perm_list.add(tuple(perm))
    return len(perm_list) == factorial(n)
 
alphabet = [1,2,3,4,5,6]
string = find_string(alphabet)
print(string)
