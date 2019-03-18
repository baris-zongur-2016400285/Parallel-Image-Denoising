/*
Student Name: Baris Zongur
Student Number: 2016400285
Compile Status: Compiling
Program Status: Working
Notes: takes beta and pi as arguments should work nice, if it doesnt i can send you my cmake file bcs i used CLION and i didnt use much comments as i explained in documentation by giving the source code
*/
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <queue>
#include <algorithm>
#include <math.h>

using namespace std;
int main(int argc, char** argv) {
    // Initialize the MPI environment
    MPI_Init(NULL, NULL);
    // Find out rank, size
    double beta = stod(argv[3]);
    double pi = stod(argv[4]);
    double gamma = 0.5*log((1-pi)/pi);
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    ofstream rezs;
    rezs.open(argv[2]);

    int N = world_size-1;
    int i, j=0, t=0;

    if (world_rank == 0) {
        queue<string> input;
        int K;
        int L;
        string length_calc;
        string working;
        if(argc != 5){ //this part reads the input and checks if it is ok
            cout<<"terminal error"<<endl;
        }
        else {
            ifstream infile(argv[1]);
            if (!infile.is_open()) {
                cout << "couldn't open file" << endl;
                return 0;
            } else {
                string s;
                while (getline(infile, s)) { // read input line by line
                    input.push(s);
                }
            }
            infile.close();
        }
        length_calc=input.front();
        length_calc.erase(std::remove(length_calc.begin(), length_calc.end(), '-'), length_calc.end());
        length_calc.erase(std::remove(length_calc.begin(), length_calc.end(), ' '), length_calc.end());
        K = length_calc.length()-1;
        printf("N: %d\n", N);
        int** arr = NULL;
        arr = (int **)malloc(sizeof(int*)*K);
        for(i = 0 ; i < K ; i++){
            arr[i] = (int *)malloc(sizeof(int) * K);
        }
        int** arr_final = NULL;
        arr_final = (int **)malloc(sizeof(int*)*K);
        for(i = 0 ; i < K ; i++){
            arr_final[i] = (int *)malloc(sizeof(int) * K);
        }
        while(!input.empty()){
            t=0;
            working = input.front();
            working.erase(std::remove(working.begin(), working.end(), ' '), working.end());
            for(i=0;i<working.length();i++){
                if(working[i]=='1'){
                    arr[j][t]= 1;
                    t++;
                }
                else{
                    arr[j][t] = -1;
                    t++;
                    i++;
                }
            }
          input.pop();
            j++;
        }
        for(i = 1 ; i <= N ; i++){
            MPI_Send(&K, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
        }
        for(i = 1 ; i <= N ; i++){
            for(j = 0 ; j < K/N ; j++){
                MPI_Send(arr[K/N*(i-1)+j], K, MPI_INT, i, j, MPI_COMM_WORLD);
            }
        }
        for(i = 1 ; i <= N ; i++){
            for(j = 0 ; j < K/N ; j++){
                MPI_Recv(arr_final[K/N*(i-1)+j], K, MPI_INT, i, j, MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            }
        }
        ofstream rezs;
        rezs.open(argv[2]);
        for(i=0;i<K;i++){
            for(j=0;j<K;j++){
                rezs<<arr_final[i][j];
                rezs<<" ";
            }
                rezs<<"\n";
        }
        rezs.close();
    }
    else{
        int** subarr = NULL;
        int myarr;
        int pixel_amount;
        MPI_Recv(&myarr, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        pixel_amount = myarr;
        subarr = (int **)malloc(sizeof(int*) * pixel_amount);
        for(i = 0 ; i < (pixel_amount/N)+2 ; i++){
            subarr[i] = (int *)malloc(sizeof(int) * pixel_amount);
        }
        for(i = 0 ; i < pixel_amount/N ; i++) {
            MPI_Recv(subarr[i+1], pixel_amount, MPI_INT, 0, i, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        for(int i=0; i<500000; i++){
            int calc_i = pixel_amount/N;
            int rand_i;
            rand_i = (rand()%calc_i)+1;
            int calc_j = pixel_amount-2;
            int rand_j;
            rand_j = (rand()%calc_j)+1;
            double same_count = 0;
            double diff_count = 0;
            double calc_rand = ((double) rand() / (RAND_MAX));
            if(world_rank==1 || world_rank == N){
                if(world_rank==1){
                    calc_i=calc_i-1;
                    rand_i = rand()%calc_i;
                    rand_i= rand_i+2;
                    MPI_Send(subarr[pixel_amount/N], pixel_amount, MPI_INT, world_rank+1, 1 , MPI_COMM_WORLD);
                    MPI_Recv(subarr[pixel_amount/N+1], pixel_amount, MPI_INT, world_rank+1, 2 , MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    for(int i= rand_i - 1; i < rand_i + 2; i++){
                        for(int j =rand_j-1; j <rand_j+2;j++ ){
                            if(i != rand_i || j != rand_j){
                                if(subarr[rand_i][rand_j]== subarr[i][j]){
                                    same_count++;
                                }
                                else{
                                    diff_count++;
                                }
                            }
                        }
                    }
                    if(calc_rand < exp(-2*gamma-2*beta*(same_count-diff_count))){
                        if(subarr[rand_i][rand_j]==1){
                            subarr[rand_i][rand_j]= -1;
                        }
                        else{
                            subarr[rand_i][rand_j]= 1;
                        }
                    }

                }
                else{
                    calc_i=calc_i-1;
                    rand_i = rand()%calc_i;
                    rand_i = rand_i+1;
                    if(world_rank%2==1){
                        MPI_Send(subarr[1], pixel_amount, MPI_INT, world_rank-1, 1 , MPI_COMM_WORLD);
                        MPI_Recv(subarr[0], pixel_amount, MPI_INT, world_rank-1, 2 , MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                        for(int i= rand_i - 1; i < rand_i + 2; i++){
                            for(int j =rand_j-1; j <rand_j+2;j++ ){
                                if(i != rand_i || j != rand_j){
                                    if(subarr[rand_i][rand_j]== subarr[i][j]){
                                        same_count++;
                                    }
                                    else{
                                        diff_count++;
                                    }
                                }
                            }
                        }
                        if(calc_rand < exp(-2*gamma-2*beta*(same_count-diff_count))){
                            if(subarr[rand_i][rand_j]==1){
                                subarr[rand_i][rand_j]= -1;
                            }
                            else{
                                subarr[rand_i][rand_j]= 1;
                            }
                        }
                    }
                    else{
                        MPI_Recv(subarr[0], pixel_amount, MPI_INT, world_rank-1, 1 , MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                        MPI_Send(subarr[1], pixel_amount, MPI_INT, world_rank-1, 2 , MPI_COMM_WORLD);
                        for(int i= rand_i - 1; i < rand_i + 2; i++){
                            for(int j =rand_j-1; j <rand_j+2;j++ ){
                                if(!(i == rand_i && j == rand_j)){
                                    if(subarr[rand_i][rand_j]== subarr[i][j]){
                                        same_count++;
                                    }
                                    else{
                                        diff_count++;
                                    }
                                }
                            }
                        }
                        if(calc_rand < exp(-2*gamma-2*beta*(same_count-diff_count))){
                            if(subarr[rand_i][rand_j]==1){
                                subarr[rand_i][rand_j]= -1;
                            }
                            else{
                                subarr[rand_i][rand_j]= 1;
                            }
                        }
                    }
                }
            }
            else{
                if(world_rank%2==1){
                    MPI_Send(subarr[1], pixel_amount, MPI_INT, world_rank + 1, 1 , MPI_COMM_WORLD);
                    MPI_Send(subarr[pixel_amount/N], pixel_amount, MPI_INT, world_rank - 1, 1 , MPI_COMM_WORLD);
                    MPI_Recv(subarr[0], pixel_amount, MPI_INT, world_rank - 1, 2 , MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    MPI_Recv(subarr[pixel_amount/N+1], pixel_amount, MPI_INT, world_rank + 1, 2 , MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    for(int i= rand_i - 1; i < rand_i + 2; i++){
                        for(int j =rand_j-1; j <rand_j+2;j++ ){
                            if(!(i == rand_i && j == rand_j)){
                                if(subarr[rand_i][rand_j]== subarr[i][j]){
                                same_count++;
                                }
                                else{
                                diff_count++;
                                }
                            }
                        }
                    }
                    if(calc_rand < exp(-2*gamma-2*beta*(same_count-diff_count))){
                        if(subarr[rand_i][rand_j]==1){
                            subarr[rand_i][rand_j]= -1;
                        }
                        else{
                            subarr[rand_i][rand_j]= 1;
                        }
                    }

                }
                else{
                    MPI_Recv(subarr[0], pixel_amount, MPI_INT, world_rank - 1, 1 , MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    MPI_Recv(subarr[pixel_amount/N+1], pixel_amount, MPI_INT, world_rank + 1, 1 , MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    MPI_Send(subarr[1], pixel_amount, MPI_INT, world_rank + 1, 2 , MPI_COMM_WORLD);
                    MPI_Send(subarr[pixel_amount/N], pixel_amount, MPI_INT, world_rank - 1, 2 , MPI_COMM_WORLD);
                    for(int i= rand_i - 1; i < rand_i + 2; i++){
                        for(int j =rand_j-1; j <rand_j+2;j++ ){
                            if(!(i == rand_i && j == rand_j)){
                                if(subarr[rand_i][rand_j]== subarr[i][j]){
                                    same_count++;
                                }
                                else{
                                    diff_count++;
                                }
                            }
                        }
                    }
                    if(calc_rand < exp(-2*gamma-2*beta*(same_count-diff_count))){
                        if(subarr[rand_i][rand_j]==1){
                            subarr[rand_i][rand_j]= -1;
                        }
                        else{
                            subarr[rand_i][rand_j]= 1;
                        }
                    }
                }
            }
        }
        for(i = 0 ; i < pixel_amount/N ; i++){
            MPI_Send(subarr[i+1], pixel_amount, MPI_INT, 0, i, MPI_COMM_WORLD);
        }
        cout<<myarr;
        printf("Process %d received elements: ", world_rank);
    }
    MPI_Finalize();
}
