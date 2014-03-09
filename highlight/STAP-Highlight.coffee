#!/usr/bin/env coffee

file = process.argv[2] ? 'data.stap' #'STAP-Highlight.coffee'

fs = require 'fs'

save = (data) ->
	fs.writeFile file+'.html', data, (err) ->
		console.log(err) if err
	data

withStyle = (f) -> 
	fs.readFile 'ptt.css', 'utf-8', (err, data) ->
		return console.log(err) if err
		f data

parse = (str) ->
	(str.split '\x1E')
	.filter( (part) -> part != '') #hard way to remove last elem
	.map (val) -> 
		splited = val.split '|', 2 #split max once
		a = splited[0].split ','   #pos, len, row, col, type
		type: Number a[4]
		pos: Number a[0]
		len: Number a[1]
		row: Number a[2]
		col: Number a[3]
		data: splited[1]

to_tag = (type, content) ->
	"<span class='ptt#{type}'>#{content}</span>"

codify = (code, style) -> 
	"<style type='text/css'>#{style}</style>
	<code><pre>#{code}</pre></code>"

merge = (arr, src) ->
	newarr = []   #arr to append to
	last = 0
	arr.forEach (tkn) ->
		if (last < tkn.pos)  #whitespace etc. solved...
			newarr.push src.substr last, tkn.pos-last
		newarr.push(to_tag tkn.type, (src.substr tkn.pos, tkn.len))
		last = tkn.pos+tkn.len
	newarr.join('')

#----------

# source = "" #original source file
# lexer = (require 'child_process')
# 	.exec './stap-lex', (err, stdout, stderr) ->
# 		return console.log(err) if err
# 		prsd = parse(stdout)
# 		console.log prsd
# 		out = merge prsd, source
# 		withStyle (style) -> save codify out, style

# fs.readFile file, 'utf-8', (err, data) ->
# 	return console.log(err) if err
# 	source = data
# 	lexer.stdin.end data

#----------- server version

http = require 'http'
cp = require 'child_process'

server = http.createServer (rqst, rsp) ->
	console.log "rqst at " + (new Date).toLocaleTimeString()

	source = "" #original source file
	rsp.writeHead 200, {"Content-Type": "text/html"}
	lexer = cp.exec './stap-lex', (err, stdout, stderr) ->
		return console.log(err) if err
		prsd = parse(stdout)
		#console.log prsd
		out = merge prsd, source
		withStyle (style) -> rsp.end save codify out, style
	
	fs.readFile file, 'utf-8', (err, data) ->
		return console.log(err) if err
		source = data
		lexer.stdin.end data
		
server.listen 8000, '127.0.0.1'
console.log "Server running at http://127.0.0.1:8000/"	 
