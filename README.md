# Practices-Operating-Sistems-2
Some projects that I´ve made with my mate Cristina ( crmanso@ucm.es ), during the subject "Ampliación de Sistemas Operativos"

Pr1: It´s the source code of a library. This library contains functións to measure the time. Useful for measuring the execution time. It´s also contains a example code that multiply two random matrices.

Pr2: Contains the code of two demons process. One ( SignalsDem ) prints in the syslog file the current time with a frequency specify in the ".tiempo.conf" file using SIGALARM. It´s also treat others kind of signals. In ( Server ) contains a demon server. This server send the current hour to a all clients with a frequency specify in the ".server.conf" file using SIGALARM. The client ( Client ), request the unix time and the server send the answer.

Pr3: This code do the same as the PR2 code but using a System V message queue.
