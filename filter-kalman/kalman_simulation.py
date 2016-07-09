# -*- coding: utf-8 -*-
import numpy as np
import numpy.matlib as mt
import matplotlib.pyplot as plt

M = 2
N = 500
dt = 1.0 / 30.
sigma_measure = 0.1
# x = 1/2 a t**2
# assume x = 0.5 in 0.5, i.e. head turns half maximal movement range in 0.5 sec.
# so a = 2 x / t**2 =  4.0
sigma_accel   = 4.0 

x = np.zeros(N, dtype = np.float)
x[N/3:] = 1.0 # step input
measurement = x.copy()
measurement += np.random.normal(0., sigma_measure, N)

A = np.matrix([
    [ 1. , dt ],
    [ 0. , 1. ]
])

R = np.matrix([[ sigma_measure**2 ]])

dv = sigma_accel * dt
dp = sigma_accel * 0.5 * dt * dt
Q = np.matrix([
    [ dp*dp, dp*dv ],
    [ dv*dp, dv*dv ]
])

H = np.matrix([
    [ 1., 0. ],
])

I = mt.identity(M, dtype = np.float)

def arrayOfMatrices(n, shape):
    return np.asarray([mt.zeros(shape, dtype = np.float) for i in xrange(n)])

# Base on the scipy-cookbook http://scipy-cookbook.readthedocs.io/items/KalmanFiltering.html
sz_state = (M, 1)
sz_cov   = (M, M)
sz_K     = (M, 1)
xhat= arrayOfMatrices(N, sz_state)      # a posteri estimate of x
P=arrayOfMatrices(N, sz_cov)         # a posteri error estimate
xhatminus=arrayOfMatrices(N, sz_state) # a priori estimate of x
Pminus=arrayOfMatrices(N, sz_cov)    # a priori error estimate
K=arrayOfMatrices(N, sz_K)         # gain or blending factor

P[0] = mt.ones((M,M)) * 100
xhat[0] = measurement[0]

for k in range(1,N):
    # time update
    xhatminus[k] = A * xhat[k-1]
    Pminus[k]    = A * P[k-1] + Q

    # measurement update
    K[k] = Pminus[k] * H.T * np.linalg.inv( H * Pminus[k] * H.T + R )
    xhat[k] = xhatminus[k] + K[k] * (measurement[k] - H * xhatminus[k])
    P[k] = ( I - K[k] ) * Pminus[k]
    
t = np.arange(N) * dt
plt.figure()
plt.subplot(2,1,1)
plt.plot(t, measurement,'k+',label='noisy measurements')
plt.plot(t, xhat[:,0,0],'b-',label='position estimate')
plt.plot(t, x, 'r-', label='ground truth')

plt.subplot(2,1,2)
plt.plot(t, xhat[:,1,0],'g-',label='velocity estimate')

plt.show()