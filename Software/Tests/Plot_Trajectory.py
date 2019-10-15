## Plot de polynomes ##
'''Permet de tracer le graphe d'un poynome quelconque sur l intervalle [0,1].
La liste des coefficients doit etre donnee de l ordre 0 a l ordre deg(polynome)
en assignant la valeur nulle a un coefficient inexistant.'''

import numpy as np
import matplotlib.pyplot as plt

def plotpolynome(coefsx, coefsy):
    def pln(x, coefs):
        y = 0
        for i in range(len(coefs)):
            y += coefs[i]*x**i
        return y

    valt=np.linspace(0, 1, 1e3)
    plt.clf()
    plt.plot([pln(k, coefsx) for k in valt],[pln(j,coefsy) for j in valt])
    plt.grid()
    plt.axis('equal')
    plt.show()


coeff1 = [0.00, 39.21, -9.68, 10.47, 0.00, 0.00, 0.00]
coeff2 = [0.00, 33.03, -21.05, 3.03, 0.00, 0.00, 0.00]

plotpolynome(coeff1, coeff2)