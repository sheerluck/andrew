# https://shironetsu.hatenadiary.com/entry/2021/05/26/003510

f(X, Y, Z) = X/(Y + Z) + Y/(Z + X) + Z/(X + Y) - 4
g(X, Y, Z) = f(X, Y, Z)*(X + Y)*(Y + Z)*(Z + X)
R = PolynomialRing(QQ, 'X,Y,Z')
phi = EllipticCurve_from_cubic(R(g))
phi_inv = phi.inverse()
[ P ] = phi.codomain().gens()
for n in range(18):
    a, b, c = phi_inv(n * P)
    k = lcm(a.denominator(), b.denominator())
    p, q, r = k*a, k*b, k*c
    if((sgn(p), sgn(q), sgn(r)) == (1, 1, 1)):
        print('\n\n>>>positive solution !<<<')
    print(n, (p, q, r), end="\n\n")
