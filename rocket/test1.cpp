#include <bits/stdc++.h>
using namespace std;

int main(){
    long long n, k;
    cin>>n>>k;
    string s;
    cin>>s;
    vector<pair<char, long long>> vp;
    int m = s.length();
    int i = 0;
    int idx = 0;    //  vp的计数
    while(i < m){
        char a = s[i];
        i += 2;
        long long count = 0;
        while(s[i]>'0' && s[i]<'9'){
            count *= 10;
            count += s[i] - '0';
        }
        vp.emplace_back(make_pair(a, count));
        ++i;
    }
    for(int i = 0; i < vp.size(); ++i){
        cout<<vp[i].first<<" "<<vp[i].second<<endl;

    }
    
    

}