// 2018321006 송민규 알고리즘 HW2

#include <iostream>
#include <cstring>
#include <climits>
#include <queue>
#include <algorithm>
#define MAX_V 501
#define MAX_SUM 62626

using namespace std;

int V[MAX_V];
long long dp[MAX_V][MAX_SUM];
bool flip = false;
bool NUMER = false;

int main() {
    
    char oper[4];
    int n;
    
    cout << "Enter input"<< '\n';
    
    Exit2:
    if(NUMER){
        cin.ignore();
        memset(V, '\0', sizeof(V));
        NUMER = false;
    }
    
    memset(V, '\0', sizeof(V));
    
    bool odd = false;
    
    while(true){
        char c = cin.peek();
        flip = false;
        
        if(isdigit(c)){
            cin >> n;
            int sum = 0;
            
            int negative = 0;
            
            for(int i=0; i<n; i++){
                cin >> V[i];
                sum += V[i];
            }
            
            if(sum % 2 == 1){
                odd = true;
            }
            
            // sum > 0 -> 0부터 그대로
            // sum < 0 -> 부호 뒤집기
            
            sum /= 2;
            if(sum<0){
                sum = -sum;
                flip = true;
                for(int i=0; i<n; i++){
                    V[i] = -V[i];
                }
            }
            
            for(int i=0; i<n; i++){
                if(V[i]<0){
                    negative += V[i];
                }
            }
            
            sort(V, V+n);
            memset(dp, '\0', sizeof(dp));
            int i,j;  // i:array, j:target sum
            
            
            // first row 초기화 필요
            for (j = 0; j < sum-negative+1; j++){
                if(negative+j == V[0])  // V[0]
                    dp[1][j] = 1;
            }
            
            // core
            for (i = 2; i <= n; i++)
            {
                for (j = 0; j < sum-negative+1; j++)
                {
                    dp[i][j] = dp[i-1][j];
                    
                    if(V[i-1] == negative+j)
                        dp[i][j]++;
                    
                    if (0 <= j-V[i-1] && j-V[i-1] < sum-negative+1)
                    {
                        dp[i][j] += dp[i-1][j-V[i-1]];
                    }
                    
                    if(dp[i][j] > UINT_MAX){
                        cout << "NUMEROUS" << '\n';
                        NUMER = true;
                        goto Exit2;
                    }
                }
            }
            
            if(sum  == 0){  // 공집합 처리
                dp[n][sum-negative]++;
            }
            
            if(odd){
                cout << 0 << '\n';
            }
            else{
                cout <<  dp[n][sum-negative]/2 << '\n';
            }
            
            
            
            int ch_idx = sum-negative;
            
            while(dp[n][ch_idx] == 0){
                ch_idx--;
            }
            
            
            string result = "{";
            int sol[MAX_V][2];
            memset(sol, '\0', sizeof(sol));
                
            for(int i=0; i<n; i++){
                if(flip){  // flip된 경우
                    sol[i][0] = -V[i];
                }else{
                    sol[i][0] = V[i];
                }
            }
            
            i = n;
            j = ch_idx;
                
            while(i>0 && j>=0){
                if(dp[i][j] != dp[i-1][j]){
                    sol[i-1][1] += 1;
                    j -= V[i-1];
                }
                i -= 1;
            }
                
            int sub1[MAX_V];
            int sub2[MAX_V];
                
            int idx1 = 0;
            int idx2 = 0;
                
            for(int i=0; i<n; i++){
                if(sol[i][1] == 1){
                    sub1[idx1++] = sol[i][0];
                }
            }
                
            for(int i=0; i<n; i++){
                if(sol[i][1] == 0){
                    sub2[idx2++] = sol[i][0];
                }
            }
                
            sort(sub1, sub1+idx1);
            sort(sub2, sub2+idx2);
                
            cout << "{";
            for(int i=0; i<idx1; i++){
                cout << sub1[i];
                if(i+1 != idx1)
                    cout << ",";
            }
            cout << "}";
                
            cout << ",{";
            for(int i=0; i<idx2; i++){
                cout << sub2[i];
                if(i+1 != idx2)
                    cout << ",";
            }
            cout << "}";
            cout << '\n';
            
            memset(V, '\0', sizeof(V));
            cin.ignore();
        }
        else if(isalpha(c)){
            cin >> oper;
            if(strcmp(oper,"EOI") == 0){
                break;
            }
        }
    }
  
    return 0;
}
