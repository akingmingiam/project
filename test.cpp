#include <iostream>
#include <cstring>
#include <chrono>
#include "datablob.hpp"
using namespace std::chrono;
using namespace std;

int main()
{
  // part1 构造函数 //
  cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
  float *a_data = new float[6]{1.8, 4.6, 3.5, 2.3, 3.2, 4.2};
  DataBlob a = DataBlob(2, 3, 1, FLOAT, a_data);
  cout << "a = DataBlob(2, 3, 1, FLOAT, a_data);" << endl
       << "a: " << a << endl;
  DataBlob b = DataBlob(2, 3, 1, FLOAT, new float[6]{1.8, 4.6, 3.5, 2.3, 3.2, 4.2});
  cout << "b = DataBlob(2, 3, 1, FLOAT, new float[6]{1.8,4.6,3.5,2.3,3.2,4.2});" << endl
       << "b: " << b << endl;
  DataBlob c = a;
  cout << "c = a;" << endl
       << "c: " << c << endl;
  DataBlob d = DataBlob(INT, -10, 10, 10, 10, 2);
  cout << "d = DataBlob(INT, -10, 10, 10, 10, 2);" << endl
       << "d: " << d << endl;
  cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;

  // part2 引用数, 随机构造函数, set, set_all, ==//
  cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
  DataBlob a1 = DataBlob(INT, -10, 10, 10, 10, 2);
  cout << "a1 = DataBlob(INT, -10, 10, 10, 10, 2);" << endl
       << "reference of a1: " << a1.get_reference_count() << endl;
  cout << "a1: " << a1 << endl;
  DataBlob a2 = a1;
  if (a2 == a1)
  {
    cout << "a2 = a1;" << endl
         << "reference of a1: " << a1.get_reference_count() << endl;
  }
  DataBlob a3 = a1.clone();
  if (a3 == a1)
  {
    cout << "a3 = a1.clone();" << endl
         << "reference of a1: " << a1.get_reference_count() << endl;
  }
  a2 = DataBlob(DOUBLE, 0, 100, 5, 5, 1);
  cout << "a2 = DataBlob(DOUBLE, 0, 100, 5, 5, 1);" << endl
       << "a2: " << a2 << endl;
  cout << "reference of a1: " << a1.get_reference_count() << endl;
  DataBlob a4 = a1;
  a4.set_all<int>(0, 0, 0, 10);
  cout << "a4 = a1; a4.set_all<int>(0,0,0,10);" << endl
       << "reference of a1: " << a1.get_reference_count() << endl;
  cout << "a1(0,0,0): " << a1.at<int>(0, 0, 0) << endl;
  a4.set<int>(0, 0, 0, 20);
  cout << "a4 = a1; a4.set<int>(0,0,0,20);" << endl
       << "reference of a1: " << a1.get_reference_count() << endl;
  cout << "a1(0,0,0): " << a1.at<int>(0, 0, 0) << endl;
  cout << "a4(0,0,0): " << a4.at<int>(0, 0, 0) << endl;
  cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;

  // part3 重载操作符与set_part get_part //
  cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
  DataBlob b1 = DataBlob(DOUBLE, -10, 10, 10, 10, 1);
  cout << "b1 = DataBlob(DOUBLE, -10, 10, 10, 10, 1);" << endl;
  cout << "b1: " << b1 << endl;
  DataBlob b2 = DataBlob(DOUBLE, -10, 10, 10, 2, 1);
  cout << "b2 = DataBlob(DOUBLE, -10, 10, 10, 2, 1);" << endl;
  cout << "b2: " << b2 << endl;
  DataBlob b3 = b1 * b2;
  cout << "b3 = b1 * b2;" << endl;
  cout << "b3: " << b3 << endl;
  b1.set_part(0, 9, 0, 1, 0, 0, b3);
  cout << "b1.set_part(0, 9, 0, 1, 0, 0, b3);" << endl;
  cout << "b1: " << b1 << endl;
  DataBlob b4 = b1.get_part(0, 3, 0, 3, 0, 0);
  cout << "b4 = b1.get_part(0, 3, 0, 3, 0, 0);" << endl;
  cout << "b4: " << b4 << endl;
  DataBlob b5 = ~(b1.get_part(0, 3, 0, 3, 0, 0));
  cout << "b5 = ~(b1.get_part(0, 3, 0, 3, 0, 0));" << endl;
  cout << "b5: " << b5 << endl;
  DataBlob b6 = b4 - ~b5;
  cout << "b6 = b4 - ~b5;" << endl;
  cout << "b6: " << b6 << endl;
  DataBlob b7 = b4 + ~b5;
  cout << "b7 = b4 + ~b5;" << endl;
  cout << "b7: " << b7 << endl;
  cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;

  // part4 运行效率 //
  double time_consuming = 0;
  for (int i = 0; i < 10; i++)
  {
    auto start = high_resolution_clock::now();
    DataBlob c1 = DataBlob(DOUBLE, 0, 10, 1000, 1000, 1);
    DataBlob c2 = DataBlob(DOUBLE, 0, 10, 1000, 1000, 1);
    DataBlob c3 = c1 + c2;
    auto stop = high_resolution_clock::now();
    time_consuming += duration_cast<microseconds>(stop - start).count();
  }

  cout << "time_consuming: " << time_consuming << "microseconds" << endl;
}
