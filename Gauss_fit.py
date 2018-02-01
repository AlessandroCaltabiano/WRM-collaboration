# -*- coding: utf-8 -*-
"""
Created on Sat Jan 27 10:53:56 2018

@author: alex
"""

import numpy as np
import matplotlib.pyplot as plt
from scipy.optimize import curve_fit

#functions for Gauss fit (with handmade initial guess)
def GaussRaw(x, A, mu, sig) :
    return (A+800)*np.exp(-((x-(20+mu))/(2*(sig+10)))**2)
def GaussSum(x, A, mu, sig) :
    return (A+1000)*np.exp(-((x-(10000+mu))/(2*(sig+100)))**2)
def GaussNormWRM(x, A, mu, sig) :
    return (A+800)*np.exp(-((x-(20+mu))/(2*(sig+10)))**2)
def GaussWRM(x, A, mu, sig) :
    return (A+4000)*np.exp(-((x-(2000+mu))/(2*(sig+100)))**2)

img=[]
#to be changed in function of the user path and name
path1="/home/alex/Desktop/PhDTV/Neutrino/wrm-neutrino/data/topologies/nuecc/"
name1="Run0Event5007_Coll_right_I_zoom.jpg"
path="/home/alex/Desktop/PhDTV/Neutrino/wrm-pytest2/Run0Event5007_Coll_right_I_zoom/"
name="20x10.npy"


Gauss=[GaussSum,GaussRaw]
img.append(np.load(path+name))      #load files produced by sum_area.py program
img.append(plt.imread(path1+name1)) #import standard image formats 


thRaw=[]
thWRM=[]
th=[thRaw,thWRM]
sigma=[]
fig=plt.figure()
for idimg,imgx in enumerate(img) :  #loop over both images
    
    if idimg==1 :                           #plot stuff
        ax1=fig.add_subplot(2,1,1)
        ax1.set_title("Raw Noise profile")
        gray_img=np.invert(imgx[:,:,0])  
    else:
        ax0=fig.add_subplot(2,1,2)
        ax0.set_title("Filtered Noise profile")
        gray_img=imgx
    xsamp,ysamp=[np.max(gray_img)*[0],np.max(gray_img)*[0]]
    for wires in gray_img.T :               #loop over columns of the image
        y,x=np.histogram(wires,bins=np.max(gray_img),range=[0,np.max(gray_img)]) #produces histos with one bin per pixel's intensity...
        ysamp+=y                    #....and sum up
        xsamp+=x[:-1]
    xsamp=xsamp/len(gray_img.T)
    
    
    
    popt,pcov=curve_fit(Gauss[idimg],xsamp,ysamp) #fit with Gauss function
    perr = np.sqrt(np.diag(pcov)) #perr=covariance matrix, diagonalize and square it in order to find sigma
    plt.xlim([0,np.max(gray_img)])
    cm=["g","b","r"]    #plot stuff
    if idimg==1 : #if is the second image (raw data)
        ax1.plot(xsamp,ysamp,label="Noise profile") #plot histos......superposed with gauss fit
        ax1.plot(xsamp, Gauss[idimg](xsamp, *popt), 'r-',label="Fit \n $\mu $="+str(round(popt[1]+20,2))+"$\pm $"+str(round(perr[1],2))+",\n $\sigma $="+str(round(popt[2]+10,2))+"$\pm $"+str(round(perr[2],2)))
        for k,n in enumerate([5,3,1]) :
            th[idimg].append(popt[1]+20+n*(popt[2]+10)) #fill vector of threshold (computed as mean+n*sigma)
            sigma.append(n*(popt[2]+10)) 
            ax1.fill_between(x=xsamp,y1=ysamp,where=(xsamp<(popt[1]+20+n*(popt[2]+10))) ,color=cm[k]) #other cool plot stuff  
                #other boring plot stuff
        ax1.annotate("${th2}$=$\mu$+$5\sigma$", xy=(78, 3200), xytext=(120, 28000),
                    arrowprops=dict(facecolor='black', shrink=0.001))
        ax1.annotate("${th1}$=$\mu$+$3\sigma$", xy=(65, 47000), xytext=(90, 81000),
                    arrowprops=dict(facecolor='black', shrink=0.001))
        ax1.annotate("${th0}$=$\mu$+$\sigma$", xy=(50, 100000), xytext=(60, 120000),
                    arrowprops=dict(facecolor='black', shrink=0.001)) 
         
    else : #same as idimg=1
        ax0.plot(xsamp,ysamp,label="Noise profile")
        ax0.plot(xsamp, Gauss[idimg](xsamp, *popt), 'r-',label="Fit \n $\mu $="+str(round(popt[1]+2000,2))+"$\pm $"+str(round(perr[1],2))+",\n $\sigma $="+str(round(popt[2]+100,2))+"$\pm $"+str(round(perr[2],2)))
        for k,n in enumerate([5,3,1]) :
            th[idimg].append(popt[1]+10000+n*(popt[2]+100))
            ax0.fill_between(x=xsamp,y1=ysamp,where=(xsamp<(popt[1]+10000+n*(popt[2]+100))) ,color=cm[k])        
           
        ax0.annotate("${th2}$=$\mu$+$5\sigma$", xy=(78, 3200), xytext=(120, 28000),
                    arrowprops=dict(facecolor='black', shrink=0.001))
        ax0.annotate("${th1}$=$\mu$+$3\sigma$", xy=(65, 47000), xytext=(90, 81000),
                    arrowprops=dict(facecolor='black', shrink=0.001))
        ax0.annotate("${th0}$=$\mu$+$\sigma$", xy=(50, 100000), xytext=(60, 120000),
                    arrowprops=dict(facecolor='black', shrink=0.001))

    plt.legend()

for i in [0,1] :
    for idt,t in enumerate(th[i]) :
        plt.figure()
        if i==0 :
            plt.title("th"+str(idt)+"after filter")
            bit=np.zeros((len(img[i]),len(img[i].T))) #zeros 2D array of same dim of img
            bit[np.where(img[i]>(t))]=1 #application of threshold, bit now is your yes/no matrix!!
            plt.imshow(bit,cmap="Greys")
        else :
            #as before
            plt.title("th"+str(idt)+" on raw data")
            imgx=np.invert(img[i][:,:,0]) 
            bit=np.zeros((len(imgx),len(imgx.T)))
            bit[np.where(imgx>t)]=1
            plt.imshow(bit,cmap="Greys")
        




