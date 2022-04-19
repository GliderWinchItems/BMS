/* polyreg.c */

#include<math.h>
#include<stdio.h>
//#include<conio.h>
main(){
   int i,j,k,m,n;
   float x[20],y[20],u,a[10],c[20][20],power,r;
   printf("enter m,n:");
   scanf("%d%d",&m,&n);
   for(i=1;i<=n;i++){
      printf("enter values of x and y");
      scanf("%f%f",&x[i],&y[i]);
   }
   for(j=1;j<=m+1;j++)
   for(k=1;k<=m+1;k++){
      c[j][k]=0;
      for(i=1;i<=n;i++){
         power=pow(x[i],j+k-2);
         c[j][k]=c[j][k]+power;
      }
   }
   for(j=1;j<=m+1;j++){
      c[j][m+2]=0;
      for(i=1;i<=n;i++){
         r=pow(x[i],j-1);
         c[j][m+2]=c[j][m+2]+y[i]*r;
      }
   }
   for(i=1;i<=m+1;i++){
      for(j=1;j<=m+2;j++){
         printf("%.2f\t",c[i][j]);
      }
      printf("\n");
   }
   for(k=1;k<=m+1;k++)
      for(i=1;i<=m+1;i++){
         if(i!=k){
            u=c[i][k]/c[k][k];
            for(j=k;j<=m+2;j++){
               c[i][j]=c[i][j]-u*c[k][j];
         }
      }
   }
   for(i=1;i<=m+1;i++){
      a[i]=c[i][m+2]/c[i][i];
      printf("a[%d]=%f\n",i,a[i]);
   }
//   getch();
}
/*
Output

When the above program is executed, it produces the following result −

enter m,n:4 5
enter values of x and y1 1
enter values of x and y2 3
enter values of x and y1 2
enter values of x and y1 2
enter values of x and y1 1
5.00  6.00  8.00  12.00  20.00  9.00
6.00  8.00  12.00 20.00  36.00  12.00
8.00  12.00 20.00 36.00  68.00  18.00
12.00 20.00 36.00 68.00  132.00 30.00
20.00 36.00 68.00 132.00 260.00 54.00
a[1]=1.750000
a[2]=-2.375000
a[3]=2.000000
a[4]=0.500000
a[5]=-0.375000

raja
*/