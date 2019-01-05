del 32to16.asm
del 32to16.obj
e:\vc\bin\thunk -t thk 32to16.thk -o 32to16.asm
e:\vc\bin\ml /DIS_32 /c /W3 /nologo /coff /Fo thk32.obj 32to16.asm 
e:\vc\bin\ml /DIS_16 /c /W3 /nologo /Fo thk16.obj 32to16.asm 

copy thk32.obj dio9x\thk32.obj
copy thk16.obj dio9x16\thk16.obj
