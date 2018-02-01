# WRM-collaboration

This folder's aim is to share software between collaborators, improve it and create new ones.
Feel free to suggest editing and changes...
All tips on codes are kindly accepted!


#wr-rd-xilly.c 
Program that uses dummy_test.txt as fake data to be trasmitted to "/dev/*_write_*" and recieve it from "/dev/*_read_*".


#sum_area.py
terminal interfacing program that performs sum over a moving window.
how to use: 
-----> type in terminal the execution command "python3 sum_area.py image_filename #len_x_window #len_y_window" for manual mode
-----> type in terminal the execution command "python3 sum_area.py image_filename run" for run mode

manual mode allow to visualize a specified filter output, if it doesn't exist it creates it.
run mode produces (not visualize) a range of values of #len_x_window #len_y_window setted up by the user.


#Gauss_fit.py
Produces noise profile of raw data and filtered data. Filtered data are refered to the output files of sum_area.py


