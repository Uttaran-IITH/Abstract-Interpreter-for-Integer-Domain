#include <interval.h>
#include <math.h>
void add(interval a,interval b,interval *c){
    c->set_lower_bound() = a.get_lower_bound() + b.get_lower_bound();
    c->set_upper_bound() = a.get_upper_bound() + b.get_upper_bound();
}
void negate(interval *a){
    int temp = a->get_upper_bound();
    a->set_upper_bound() = -(a->get_lower_bound());
    a->set_lower_bound() = -temp;
}
void sub(interval a,interval b, interval *c){
    negate(&b);
    add(a,b,c);
}
void multiply(interval a,interval b, interval *c){

}
void power(interval *a, int p){
    a.set_lower_bound() = pow(a.get_lower_bound(),p);
    a.set_upper_bound() = pow(a.get_upper_bound(),p);
}