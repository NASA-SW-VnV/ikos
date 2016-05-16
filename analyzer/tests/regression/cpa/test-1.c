int foo(int a, int b) {
  int i, j, k;
  int x;
  int tmp0, tmp1, tmp2, tmp3, temp1, temp2, temp3, temp4, temp5;
  int T0, T1;

  tmp0 = a - b;
  temp1 = tmp0; // CAN BE REMOVED AFTER CPA  {(tmp0,a-b)}
  for (i = 0; i < 100; i++) {
    tmp1 = temp1; // CAN BE REMOVED AFTER CPA  {}
    temp2 = tmp1; // <---                      {(tmp1,temp1)}
    temp3 = 0;
    for (j = 0; j < 100; j++) {
      tmp2 = temp2; // CAN BE REMOVED AFTER CPA {}
      tmp3 = temp3; // CAN BE REMOVED AFTER CPA {(tmp2,temp2)}
      temp4 = 0;    //                          {(tmp2,temp2); (tmp3,temp3)}
      temp5 = tmp2; // <--                      {(tmp2,temp2); (tmp3,temp3);
                    // (temp4,0)}
      for (k = 0; k < 100; k++) {
        T0 = temp4;  // CAN BE REMOVED AFTER CPA  {}
        T1 = temp5;  // CAN BE REMOVED AFTER CPA  {(T0,temp4)}
        x = T0 + T1; // <---                      {(T0,temp4); (T1,temp5)}
        temp4 = temp4 +
                tmp3; // <---             {(x,T0 + T1); (T0,temp4); (T1,temp5)}
        temp5 = temp5 + 4; //                 {(x,T0 + T1); (temp4,temp4 +
                           //                 tmp3); (T1,temp5)}
      }
      temp2 = temp2 + 400;
      temp3 = temp3 + i;
    }
    temp1 = temp1 + 40000;
  }
}

/*

// After DCE and CPA
int foo(int a, int b){

  int i, j, k;
  int tmp0, tmp1, tmp2, tmp3, temp1, temp2, temp3, temp4, temp5;
  int T0, T1;

  tmp0 = a - b;
  temp1 = tmp0;
  for (i=0; i<100;i++){
    temp2 = temp1
    temp3 = 0;
    for (j=0; j<100;j++){
      temp4 = 0;
      temp5 = temp2;
      for (k=0; k<100;k++){
        x = temp4 + temp5;
        temp4 = temp4 + temp3;
        temp5 = temp5 + 4;
      }
      temp2 = temp2 + 400;
      temp3 = temp3 + i;
    }
    temp1 = temp1 + 40000;
  }

}
*/
