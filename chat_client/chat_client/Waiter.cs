using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace chat_client
{
    delegate object method_obj1(object obj1);

    class Waiter{
        public object res;
        public Waiter(ref UInt64 first, ref UInt64 second, method_obj1 m, object obj1){
            while (first != second) { System.Threading.Thread.Sleep(5); }
            ++first;
            res = m(obj1);
            second++;
        }
        public Waiter(ref UInt64 first, ref UInt64 second, method_empty m){
            while (first != second) { System.Threading.Thread.Sleep(5); }
            ++first;
            m();
            second++;
        }
    }
}
