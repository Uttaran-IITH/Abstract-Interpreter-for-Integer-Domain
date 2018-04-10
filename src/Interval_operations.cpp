#include <interval.h>
#include <math.h>
#include <stdlib.h>
void add(interval a,interval b,interval *c){
    c->set_lower_bound(a.get_lower_bound() + b.get_lower_bound());
    c->set_upper_bound(a.get_upper_bound() + b.get_upper_bound());
}
void negate(interval *a){
    int temp = a->get_upper_bound();
    a->set_upper_bound(-(a->get_lower_bound()));
    a->set_lower_bound(-temp);
}
void sub(interval a,interval b, interval *c){
    negate(&b);
    add(a,b,c);
}
void multiply(interval a,interval b, interval *c){
    int temp[] = {
               a.get_lower_bound() * b.get_lower_bound(), 
               a.get_lower_bound() * b.get_upper_bound(),
               a.get_upper_bound() * b.get_lower_bound(),
               a.get_upper_bound() * b.get_upper_bound()
               };
    int min =temp[0];
    int max = temp[0];
    for(int i=1;i<4;i++){
        if(min > temp[i])
            min = temp[i];
        if (max < temp[i])
            max = temp[i];
    }
    c->set_lower_bound(min);
    c->set_upper_bound(max);
}
void power(interval *a, int p){
    int sign[]={1,1};
    if(a->get_lower_bound()<0)
        sign[0]=-1;
        if(a->get_upper_bound()<0)
            sign[1]=-1;
    a->set_lower_bound(sign[0] * abs(pow(a->get_lower_bound(),p)));
    a->set_upper_bound(sign[1] * abs(pow(a->get_upper_bound(),p)));
}
void less_than(interval *a, interval *b){
    int l1 = a->get_lower_bound();
    int u1 = a->get_upper_bound();
    int l2 = b->get_lower_bound();
    int u2 = b->get_upper_bound();
    if(u1<l2){
        return;
    }
    else if (u2<l1){
        a->set_lower_bound(NULL);
        a->set_upper_bound(NULL);
        return;
    }
    else{
        if(l1<l2)
            a->set_lower_bound(l1);
        else
            a->set_lower_bound(l2);
        if(u1<u2)
            a->set_upper_bound(u1);
        else
            a->set_upper_bound(u2);
    }
}