#include <time.h>
#include <map>
#include <iostream>
using namespace std;
template<class Tkey, class Tvalue>
class CacheCfg
{
public:
    CacheCfg();
    CacheCfg(time_t timeout);

    //return 0 when eixst and valid, 1 when not exist, 2 when expired
    int get(const Tkey &key, Tvalue &value) const;
    void set(const Tkey &key, const Tvalue &value);
private:
    typedef struct _Tcontent
    {
        Tvalue value;
        time_t expire;
    }Tcontent;
    map<Tkey, Tcontent> content;
    time_t timeout; 
};


template<class Tkey, class Tvalue>
CacheCfg<Tkey, Tvalue>::CacheCfg(time_t iTimeout):timeout(iTimeout){}


template<class Tkey, class Tvalue>
CacheCfg<Tkey, Tvalue>::CacheCfg():timeout(3600)
{
};


template<class Tkey, class Tvalue>
void CacheCfg<Tkey, Tvalue>::set(const Tkey &key, const Tvalue &value)
{
    Tcontent con;
    con.value = value;
    con.expire = (time(NULL) + timeout); 
    content[key] = con;
    return;
};

template<class Tkey, class Tvalue>
int CacheCfg<Tkey, Tvalue>::get(const Tkey &key, Tvalue &value) const 
{
    typename map<Tkey, Tcontent>::const_iterator cit = content.find(key);
    if (cit != content.end())
    {
        const Tcontent & con = cit->second;
        cout << con.expire << endl;
        cout << time(NULL) << endl;
        cout << timeout << endl;
        if (con.expire > time(NULL))
        {
            value = con.value;
            return 0;
        }
        else
        {
            return 2; 
        }
    }
    else
        return 1;
}
