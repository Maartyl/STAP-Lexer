#!/usr/bin/env coffee

file = process.argv[2] ? 'data.stap' #'STAP-Highlight.coffee'

fs = require 'fs'

save = (data) ->
	fs.writeFile file+'.html', data, (err) ->
		console.log(err) if err

withStyle = (f) -> 
	fs.readFile 'ptt.css', 'utf-8', (err, data) ->
		return console.log(err) if err
		f data

parse = (str) ->
	(str.split '\x1E')
	.filter( (part) -> part != '\n') #hard way to remove last elem
	.map (val) -> 
		splited = val.split '|', 2 #split max once
		a = splited[0].split ','   #pos, len, row, col, type
		type: a[4]
		pos: a[0]
		len: a[1]
		row: a[2]
		col: a[3]
		data: splited[1]

to_tag = (type, content) ->
	if type == -1 then "" else
	"<span class='ptt#{type}'>#{content}</span> "

codify = (code, style) -> 
	"<style type='text/css'>#{style}</style>
	<code><pre>#{code}</pre></code>"

merge = (arr, src) ->
	newarr = []   #arr to append to
	last = 0
	arr.forEach (tkn) ->
		#if (last < pos)  ###whitespace etc. not solved...
		#newarr.push(to_tag 0, (src.substring last, pos)) 
		newarr.push(to_tag tkn.type, (src.substr tkn.pos, tkn.len))
		last = tkn.pos+tkn.len-1
	newarr.join ''

#----------

source = "" #original source file
lexer = (require 'child_process')
	.exec './stap-lex', (err, stdout, stderr) ->
		return console.log(err) if err
		prsd = parse(stdout)
		console.log prsd
		out = merge prsd, source
		withStyle (style) -> save codify out, style

fs.readFile file, 'utf-8', (err, data) ->
	return console.log(err) if err
	source = data
	lexer.stdin.end data
