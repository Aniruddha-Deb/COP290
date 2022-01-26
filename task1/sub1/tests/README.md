Dear Students,

To aid in smooth development and evaluation of this assignment, we are providing some updates regarding the SubTask1 (of Assignment1).

You are expected to follow these updates which are inline with the original assignment posted @ https://www.cse.iitd.ac.in/~rijurekha/cop290_2022.html

1) Various parts of your subtask should be executed on demand via the control parameters passed to your executable (i.e. the .out). Your code should give appropriate results for -

```
./yourcode.out fullyconnected inputmatrix.txt weightmatrix.txt biasmatrix.txt outputmatrix.txt
./yourcode.out activation relu inputmatrix.txt outputmatrix.txt
./yourcode.out activation tanh inputmatrix.txt outputmatrix.txt
./yourcode.out pooling max inputmatrix.txt stride outputmatrix.txt
./yourcode.out pooling average inputmatrix.txt stride outputmatrix.txt
./yourcode.out probability softmax inputvector.txt outputvector.txt
./yourcode.out probability sigmoid inputvector.txt outputvector.txt
```

2) As you could note, the asked output files should be created by your code and the results should be saved in the same format as for input files. You can use console for debugging/informative purposes. Any thing printed on the console will be ignored.

3) As the given matrix / vector require proper dimensions to be extracted from the input files, a slight update is made to the file format. 

For matrix files (for functions 1-3), the first line will contain an integer denoting the column dimension, and the second line will denote row dimension. From the third line onwards, single float values are given in column major order in each line of the file.

Similarly, for vector files (for function 4), the first line will contain an integer denoting the length, and from the second line onwards, single float values are given in each line of the file.

4) Test Cases are provided in Subtask1TestCases.zip, for both for input and output validation.

All filenames start with "a" denoting the subtask1. 

The second character goes from 1 to 4, denoting the function (fullyconnected/activation/pooling/probability) for which this file is used for a test case.

The third character is an alphabet starting "a", and denotes a specific test case.

So, a3b is the second (b) test case for pooling (3rd) function for subtask1 (a).

The following names in the test cases are self explanatory.

5) A special file named type.txt or typeStride.txt tells about the other special arguments needed by the target function for this test case. For pooling test cases, the size of the matrix will be a multiple of the stride (given in typeStride.txt).

6) A SUMMARY.txt file enlists the data from all the test cases in a simple and readable format. You can refer this to understand/analyze the test cases.

7) You should post your related doubts in this post only as follow-up (unless something requires a private msg).
We would also appreciate if fellow students could clarify them (before we reach out).
We will update this main post (in bold) if any clarification is of general significance.

Submit the subtask1 @ https://moodle.iitd.ac.in/mod/assign/view.php?id=58796

Have fun !!!
