s = 'Root: HKCR; Subkey: "{#MyAppExeName}.[EXTENSION]"; Flags: uninsdeletekey\n' + \
	'Root: HKCR; Subkey: "{#MyAppExeName}.[EXTENSION]"; ValueType: string; ValueName: ""; ValueData: "[NAME] file"\n' + \
	'Root: HKCR; Subkey: "{#MyAppExeName}.[EXTENSION]\\DefaultIcon"; Flags: uninsdeletekey\n' + \
	'Root: HKCR; Subkey: "{#MyAppExeName}.[EXTENSION]\\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\\bin\\{#MyAppExeName}"\n' + \
	'Root: HKCR; Subkey: "{#MyAppExeName}.[EXTENSION]\\shell\\open\\command"; Flags: uninsdeletekey\n' + \
	'Root: HKCR; Subkey: "{#MyAppExeName}.[EXTENSION]\\shell\\open\\command"; ValueType: string; ValueName: ""; ValueData: """{app}\\bin\\{#MyAppExeName}"" ""%1"""\n' + \
	'Root: HKCR; Subkey: "Applications\{#MyAppExeName}\SupportedTypes"; ValueType: string; ValueName: ".[EXTENSION]"  ; ValueData: ""\n' + \
	'Root: HKCR; Subkey: ".[EXTENSION]\OpenWithProgIds"; ValueType: string; ValueName: "{#MyAppExeName}.[EXTENSION]"  ; ValueData: ""\n'

array = [
	'bmp',
	'jpg',
	'jpeg',
	'png',
	'gif',
	'svg',
	'webp',
	'ico',
	'tga',
	'tif',
	'tiff',
	'jp2'
]

for i in array:
	ext = i
	name = i.upper()
	print(s.replace('[EXTENSION]', ext).replace('[NAME]', name))
