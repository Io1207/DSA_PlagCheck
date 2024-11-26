int NchooseK(int n, int k){
    if(k == 0)return 1;
    int NchooseK(int n, int k);
    if(n == k)return 1;
    else
        return NchooseK(n-1, k-1) + 
            NchooseK(n-1, k);
    int i = 0;
}