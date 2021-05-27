int func1 (int a, int b, int c)
{
  int i;
  int ret = 0;
  for (i = a; i < b; i++) {
   c = c+1;
  }

  for (i = a; i<b; i++){
   c = c+2;
  }
  return ret + c;
}

int func2 (int a, int b, int c){
 int ret = 0;
 int i,j,k;
 for(i=0; i<10; i++){
   int d = 0;
   if(d<10){
     d++;
   }else{
     d--;
   }
   int atm = 0;
   __sync_fetch_and_add(&atm, 0x00000001);
   for(j=0; j<20; j++){
     for(k=0; k<10; k++){
      c = c+i+j+k;
     }
   }
 }
 return ret+c;
}

int func3(int a, int b){
  int ret = 0;
  int i,j,k;
  for(i=0; i<10; i++){
    for(j=0; j<10; j++){
      ret = ret + i + j;
    }
  }
  for(i=0; i<10; i++){
    for(j=0; j<10; j++){
      for(k=0; k<10; k++){
        ret = ret + i + j - k;
	int atm = 0;
	__sync_fetch_and_add(&atm, 0x00000001);
      }
    }
  }
  return ret;
}

int main(){
  func1(1,2,3);
  func2(2,3,4);
  return 0;
}
