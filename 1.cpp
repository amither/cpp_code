#include <iostream>
void put(int i)
{
    std::cout<<i<<std::endl;
}
int main()
{
    int i = 1;
    int &j = i;
    put(j);
    return 0;
}
    
16    
16    
16    

