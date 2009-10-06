def nixify(path):
	return path.replace('\\', '/')

def array_remove(array, to_remove):
	if to_remove in array:
		array.remove(to_remove)

class scoped_cmd:
	def __init__(self, cmd):
		self.cmd = cmd
	def __del__(self):
		self.cmd()

def get_lcid(lang):
	from locale import windows_locale

	lang = lang.replace('-', '_')

	# look for an exact match
	for (id, name) in windows_locale.iteritems():
		if name == lang:
			return id

	# ignore the "sub-language" part
	lang = lang.split('_')[0]
	for (id, name) in windows_locale.iteritems():
		if name.split('_')[0] == lang:
			return id

	return 0x409 # default: en-US

def get_win_cp(lcid):
	import ctypes

	LOCALE_IDEFAULTANSICODEPAGE = 0x1004
	LOCALE_RETURN_NUMBER = 0x20000000

	buf = ctypes.c_int()
	ctypes.windll.kernel32.GetLocaleInfoA(lcid, LOCALE_RETURN_NUMBER | LOCALE_IDEFAULTANSICODEPAGE, ctypes.byref(buf), 6)

	if buf.value != 0:
		return 'cp' + str(buf.value)
	return 'cp1252'

def html_to_rtf(string):
	# escape chars: \, {, }
	# <br/> -> \line + remove surrounding spaces + append a space
	# remove double new lines + remove new lines at beginning and at end
	# <b>...</b> -> {\b ...}
	# <i>...</i> -> {\i ...}
	# <u>...</u> -> {\ul ...}
	import re
	line = r'\\line '
	return re.sub('<([bi])>', r'{\\\1 ', re.sub('</[biu]>', '}',
		re.sub('^(' + line + ')', '', re.sub('(' + line + ')$', '',
		re.sub('(' + line + ')+', line, re.sub('\s*<br ?/?>\s*', line,
		string.replace('\\', '\\\\').replace('{', '\\{').replace('}', '\\}'))))))).replace('<u>', '{\\ul ')
