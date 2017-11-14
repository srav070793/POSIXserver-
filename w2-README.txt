Documentation for Warmup Assignment 2
=====================================

+-------+
| BUILD |
+-------+
type 'make' will create the require executables for running the program. 

+---------+
| GRADING |
+---------+
Basic running of the code : +100 points (+10 points for each run
100/100- All testcases passed
        
+---------------+
| Minus points: |
+---------------+

Missing/incomplete required section(s) in README file : -20 points (even if lots of comments in code): None

Submitted binary file : -2 points :No binary is submitted

Cannot compile      : -5 to -10, depending on effort to make it work: Just type 'make'

Compiler warnings   : -1 to -3
                      None, 

"make clean" : -2 points if cannot delete all binary files created during compilation
               All the executables are deleted.

Segmentation faults : -10 points
                No seg faults
Separate compilation : -10 points
                No code is write in .h files. Separate files are created and linked using the linker.

Delay trace printing : -30 points
                      There is some delay. But its not deadlock. I couldnt figure out why.

Using busy-wait : -30 points
                       ./warmup2
                       No busy waiting. Used condition variables whenever required.

Handling of commandline arguments:
        All the commandline arguments are handled perfectly

Trace output : -46 points
    Trace output is right. It matches the sample output given in the spec and satisfies the requirements given in the grading guidelines.

Statistics output : -24 points
        I dont know if my statistics are right. I implemented the logic given for statistics in the spec.

Output bad format : -5 points
    All the timestamps are lined up!

Output wrong precision for statistics (should be 6-8 significant digits) : -3 points: Correct precision is used.".6g" format specifier as instructed in the spec.

Large service time test : -3 points
    Handled properly. It(1/mu) cant be more than 10 seconds according to the spec. 
Large inter-arrival time test : -3 points
    Handled appropriately. Again cant be more than 10 secs
Tiny inter-arrival time test : -3 points
    Done.
Tiny service time test : -3 points
    Didnot test
Large total number of customers test : -5 points
    (press <Cntrl+C> after 15 seconds)
    Done
Large total number of customers with high arrival rate test : -5 points
    (press <Cntrl+C> after 5 seconds)
    Didnot test the case.

Cannot handle <Cntrl+C> at all (ignored or no statistics) : -20 points: Cntrl+C is handled perfectly.

Cannot stop token depositing thread when required : -3 points
    Token thread stops when last packet is out o the system.

Not using condition variables or do some kind of busy-wait : -15 points
Condition variable used. No busy waiting

Synchronization check : -20 points
    All threads are synchronized. Mutex variable used whenever required.

Deadlocks : -10 points: No deadlocks

Bad commandline or command : -1 point each for each not handling properly: All the commandline errors are handled correctly.

+------+
| BUGS |
+------+
No bugs according to the spec described. All the requirements to solve the problem are implemented successfully.

Comments on design decisions: 
Used single mutex variable to lock all the shared variables whenever required.
Contains six threads-4+a parent thread(main function)+ thread for handling cntrl+c

Comments on deviation from spec: 
No deviations.
