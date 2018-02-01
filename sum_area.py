# -*- coding: utf-8 -*-
"""
Created on Sat Jan  6 20:17:11 2018

@author: alex
"""
import math
import matplotlib.pyplot as plt
import numpy as np
import sys
import os
from scipy import misc

def Diffuse(DATA):

    data_nrow, data_ncol = DATA.shape

    DATA_LOCAL = np.zeros((data_nrow, data_ncol))
    DATA_LOCAL[:] = DATA

    K1 = [math.pow(1/2,np.abs(n)) for n in range(-int((data_nrow)/2.), int((data_nrow)/2.))]
    K2 = [math.pow(1/2,np.abs(n)) for n in range(-int((data_ncol)/2.), int((data_ncol)/2.))]
    
    K1,K2 =np.meshgrid(K1,K2)    
    return (K1.T+K2.T)/2.

def Diff_Mask_sum(img,mask,x_sample,y_sample) :
    """perform sum over img with  a mask sampling image with steps x,y_samples"""
    Psum=[]
    TotalSum=[]
    for i in np.arange(0,len(img)-len(mask),y_sample) :
        TotalSum.append(Psum)
        for j in np.arange(0,len(img.T)-len(mask.T),x_sample) :
           Psum.append(np.sum(np.sum(Diffuse(mask)*img[i:i+len(mask),j:j+len(mask.T)],axis=0))) 
        Psum=[]
    return TotalSum

def Norm_Mask_sum(img,mask,x_sample,y_sample) :
    """perform sum over img with  a mask sampling image with steps x,y_samples"""
    Psum=[]
    TotalSum=[]
    for i in np.arange(0,len(img)-len(mask),y_sample) :
        TotalSum.append(Psum)
        for j in np.arange(0,len(img.T)-len(mask.T),x_sample) :
           Psum.append(np.mean(np.mean(img[i:i+len(mask),j:j+len(mask.T)],axis=0))) 
        Psum=[]
    return TotalSum


def Mask_sum(img,mask,x_sample,y_sample) :
    """perform sum over img with  a mask sampling image with steps x,y_samples"""
    Psum=[]
    TotalSum=[]
    for i in np.arange(0,len(img)-len(mask),y_sample) :
        TotalSum.append(Psum)
        for j in np.arange(0,len(img.T)-len(mask.T),x_sample) :
           Psum.append(np.sum(np.sum(img[i:i+len(mask),j:j+len(mask.T)],axis=0))) 
        Psum=[]
    return TotalSum

def MakeSave(data,dirname,filename) :    #Save data in dirname in a binary filename
    cwd=os.getcwd()
    data_path=cwd+'/'+dirname+'/'
    if not(os.path.isdir(data_path)) :
        os.mkdir(data_path)
        np.save(data_path+filename,data)
    elif not os.path.isfile(data_path+filename) :
        np.save(data_path+filename,data)
    
    return True

def BitMap(fimg) : 
    """BitMap return yes/no matrix according to th guessed (filtered data)"""
    th=14700
    fimg=np.array(fimg)
    bitmap=np.zeros((len(fimg),len(fimg.T)))
    bitmap[np.where(np.array(fimg)>th)]=1
    print(np.max(fimg),np.min(fimg))
    return bitmap

def TBitMap(simg) :
    """TBitMap return yes/no matrix according to th guessed (raw data)"""
    th=110
    bitmap=np.zeros((len(simg),len(simg.T)))
    bitmap[np.where(np.array(simg)>th)]=1
    print(np.max(simg),np.min(simg))
    return bitmap

    
def plot(img,fimg,bimg,thimg) : #all in one plot!
    ax1=plt.subplot(2,1,2)
    ax1.set_title("filter output")
    ax1.set_xlabel("# wires")
    ax1.imshow(fimg,interpolation='None',cmap='Greys')
    ax2=plt.subplot(2,1,1)
    ax2.set_title("Raw data")
    ax2.set_ylabel("Samples [400ns]")
    ax2.imshow(img,interpolation='None',cmap='Greys')
    """ax3=plt.subplot(2,3,3)
    ax3.set_title("After Threshold")    
    ax3.imshow(bimg,interpolation='None',cmap='Greys')
    ax4=plt.subplot(2,3,4)
    ax4.set_title("Direct Threshold")
    ax4.imshow(thimg,interpolation='None',cmap='Greys')""" #enable for visualize threshold output
    plt.show()
    return True

def singlePlot(img,fimg,bimg,thimg) : #all different plots
    plt.figure(0)
    plt.title("Filtered output")
    plt.xlabel("# wires")
    plt.ylabel("Samples [400ns]")
    plt.imshow(fimg,interpolation='None',cmap='Greys')
    plt.figure(1)
    plt.title("Raw data")
    plt.xlabel("# wires")
    plt.ylabel("Samples [400ns]")
    plt.imshow(img,interpolation='None',cmap='Greys')
    """plt.figure(2)
    plt.xlabel("# wires")
    plt.ylabel("Samples [400ns]")
    plt.title("Threshold on WRM-like Output")    
    plt.imshow(bimg,interpolation='None',cmap='Greys')
    plt.figure(3)
    plt.xlabel("# wires")
    plt.ylabel("Samples [400ns]")
    plt.title("Threshold on Raw Data")
    plt.imshow(thimg,interpolation='None',cmap='Greys')""" #enable for visualize threshold output
    plt.show()
    return True

if __name__=="__main__" :
    if (len(sys.argv)!=4)and(len(sys.argv)!=3) : 
        #argument passed must be 3 for manual mode:
        #argv[1] name of the image
        #argv[2] horizontal size
        #argv[3] vertical size
        #argument passed must be 2 for run mode:
        #argv[1] name of the image
        #argv[2] "run" string for run mode
        print("no argument given")
        print(sys.argv)
        sys.exit(-1)
    else :
        if (len(sys.argv)==4) :
            print("manual mode:")
            try:
                cwd=os.getcwd()
                path="/"+sys.argv[1][:-4]+"/"+sys.argv[2]+"x"+sys.argv[3] #path+filename to be saved
                if not os.path.isfile(cwd+path+".npy") : #if file already exist create it
                    print("info: creating "+path+".npy")
                    mask=np.zeros((int(sys.argv[2]),int(sys.argv[3]))) #window size
                    img=np.invert(misc.imread(sys.argv[1])[:,:,0])     #import image
                    pass_img=Mask_sum(img,mask,1,1)                    #sum algorithm
                    MakeSave(pass_img,sys.argv[1][:-4],sys.argv[2]+"x"+sys.argv[3]) #save as binary file
                else :  #if file already exist....
                    print("info: file already exist")
                    pass_img=np.load(cwd+path+".npy") #....load it.....
                    img=np.invert(misc.imread(sys.argv[1])[:,:,0])#....load old image...
                    
                   
            except BaseException as e :
                print("Error reading filename "+str(e))            
            finally :
                    #plot(img,pass_img,BitMap(pass_img),TBitMap(img))
    	        singlePlot(img,pass_img,BitMap(pass_img),TBitMap(img))#...plot raw and filter image
        elif(sys.argv[2]=="run"):
            """run mode: will produce all filter results of all combinations of window size
               from (xa,ya) to (xb,yb)"""
            print("automatic mode:")
            print("horizontal window size:")
            xa=input("run from...: ")
            print(xa,type(xa),type(int(xa)))
            xb=input("to...: ")
            print("vertical window size:")
            ya=input("run from...: ")
            yb=input("to...: ")
            try:
                cwd=os.getcwd()
                for x in np.arange(int(xa),int(xb)) :
                    for y in np.arange(int(ya),int(yb)) :
                        path="/"+sys.argv[1][:-4]+"/"+str(x)+"x"+str(y) #path+filename to be saved
                        if not os.path.isfile(cwd+path+".npy") : #if file already exist create it
                            print("info: creating "+path+".npy")
                            mask=np.zeros((x,y)) #window size
                            img=np.invert(misc.imread(sys.argv[1])[:,:,0])     #import image
                            pass_img=Mask_sum(img,mask,1,1)                    #sum algorithm
                            MakeSave(pass_img,sys.argv[1][:-4],str(x)+"x"+str(y)) #save as binary file
                        else :
                            print("info: file already exist")
                                                                  
            except BaseException as e :
                print("Error reading filename "+str(e))            
