#!python
env = Environment()
env.Append(CPPPATH = [
	'/usr/include/', 
	'/usr/include/gtk-3.0/',
  '/usr/include/glib-2.0/',
	'/usr/local/lib/glib-2.0/include/',
  '/usr/local/include/poppler/glib/',
  'src/'
])
env.Append(CCFLAGS=['-w'])
env.ParseConfig('pkg-config --cflags --libs gtk+-3.0 poppler-glib')
env.Append(LIBPATH = ['src/'])
Copy("assets", "assets")
env.Program(target='picture-box', source=['src/main.c', 'src/picture.c', 'src/file-select.c', 'src/pdf.c'])