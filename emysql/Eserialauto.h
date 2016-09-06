
#ifndef EMYSERIALAUTO_H
#define EMYSERIALAUTO_H


#define symFILE "/espsql1.ini"
#define symFILEFLAG "/espsql1.upd"
#define symSepA ","
#define symSepB "#$"
#define symPar  "%"

#define MaxSqlStr 10


//unsigned long ESP.getCycleCount()



 



void saveini(void);
int lp(String in,String dv);
String ex(String in,String dv,int idx);
String getfromfile(int sqlnum, int row, int col);
/*void atPar(String cmdSQL);
void atSql(String cmdSQL);
void atExe(String cmdSQL);
void atRes(String cmdSQL);
void atDeb(String cmdSQL);
void atClr(String cmdSQL);*/
void SerialAutoMakerLoop();
void mysdelay(unsigned long ddd);
void storestatsql(int s, int cd, int c, int r);
void SaveStat(int x,int z);
bool ifreconnect(void);

#endif
