#!/usr/bin/env coffee

file = process.argv[2] ? 'data.stap'#'STAP-Highlight.coffee'

fs = require 'fs'

style = ""
fs.readFile 'ptt.css', 'utf-8', (err, data) ->
	console.log(err) if err
	style = data


save = (data) ->
	fs.writeFile file+'.html', data, (err) ->
		console.log(err) if err

parse = (str) ->
	arr = str.split '\x1E'
	arr = arr.filter (part) -> part != '\n' #hard way to remove last elem
	arr.map (val) -> 
		splited = val.split '|', 2
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

codify = (code) -> 

	"<style type='text/css'>#{style}</style>
	<code><pre>#{code}</pre></code>"

# merge = (arr, src) ->
# 	source = src
# 	offs = 0 #offset
# 	arr.forEach (tkn) ->
# 		pos = tkn.pos + offs
# 		len = tkn.len
# 		orig = source.substr(pos, len)
# 		tag = to_tag tkn.type, orig
# 		source = (source.substr 0, pos) + tag +
# 				 (source.substr(pos + len))
# 		offs += (tag.length - orig.length)
# 		console.log "tag #{tag.length} - orig #{orig.length}: #{tag.length - orig.length}"
# 	source

merge2 = (arr, src) ->
	source = src
	newarr = []   #arr to append to
	last = 0
	arr.forEach (tkn) ->
		pos = tkn.pos
		len = tkn.len
		#if (last < pos)  
		#newarr.push(to_tag 0, (src.substring last, pos)) 
		newarr.push(to_tag tkn.type, (src.substr pos, len))
		last = pos+len-1

	newarr.join ''

#----------

lexer =
	(require 'child_process')
	.spawn './stap-lex'

source = "" #original source file
buff = ""	#is being read by chunks, could be problem in split

#lexer events
lexer.stdout.setEncoding 'utf-8'
lexer.stdout.on 'data', (data) ->
	buff += data
lexer.on 'close', ->
	out = merge2 parse(buff), source
	save codify out


fs.readFile file, 'utf-8', (err, data) ->
	console.log(err) if err
	source = data
	lexer.stdin.end data
