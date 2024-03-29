#!/usr/bin/env node

const ecc = require('eosjs-ecc')
var args = process.argv.splice(2)
var cmd = args[0]

function char2symbol(d)
{
	c = d.charCodeAt(0)
	if(c >= 'a'.charCodeAt(0) && c <= 'z'.charCodeAt(0))
		return BigInt((c - 'a'.charCodeAt(0)) + 6);
	if(c >= '1'.charCodeAt(0) && c <= '5'.charCodeAt(0))
		return BigInt((c - '1'.charCodeAt(0)) + 1);
	return BigInt(0);
}
function str2name(str)
{
	var n = BigInt(0);
	for(var i = 0; str[i] && i < 12; i ++)
	{
		var ab = (char2symbol(str[i]) & BigInt(0x1f)) << BigInt(64 - 5 * (i + 1))
 		n |= (char2symbol(str[i]) & BigInt(0x1f)) << BigInt(64 - 5 * (i + 1))
	}
	return n;
}

function serialize(b)
{
	var ib = b.match(/(.{1,2})/g).reverse().join('')
	var buff = Buffer.concat([Buffer.from( b, 'hex'), Buffer.from('000000000000000000000000000000000000000000000000','hex')], 32)
	return buff;
}

function serialize2(b)
{
	var ib = b.match(/(.{1,2})/g).reverse().join('')
	var buff = Buffer.concat([Buffer.from(ib, 'hex')], 32);
	return buff;
}

function fastid(eosname)
{
	var name = eosname
	var identity = str2name(name);
	var hash = ecc.sha256(serialize(identity.toString(16)));
	var idhash = ecc.sha256(serialize2(hash))

	//var ib = idhash.match(/(.{1,2})/g).reverse().join('')
	var buff = Buffer.concat([Buffer.from(idhash, 'hex')], 32);
	var serial = BigInt(0);
	serial =  (BigInt(buff[0]) << BigInt(56))
			+ (BigInt(buff[4]) << BigInt(48))
			+ (BigInt(buff[8]) << BigInt(40))
			+ (BigInt(buff[12]) << BigInt(32))
			+ (BigInt(buff[16]) << BigInt(24))
			+ (BigInt(buff[20]) << BigInt(16))
			+ (BigInt(buff[24]) << BigInt(8))
			+ (BigInt(buff[28]) << BigInt(0))
	// console.log(serial.toString());
	return serial.toString();
}

function to256hex(n)
{
	var str = BigInt(n).toString(16);
	var ret = '0'.repeat(64 - str.length) + str;
	return ret;
}

if(!cmd)
{
	console.log("Usage: fastevm <command> <param...>");
	process.exit(0)
}

switch(cmd)
{
	case "getfastid":
	{
		if(args.length != 2)
		{
			console.log("Usage: fastevm getfastid <account>")
			console.log("Example: fastevm getfastid gnufoognufoo")
			process.exit(0);
		}
		var fid = fastid(args[1])
		console.log(fid)
		break;
	}

	case "transferto":
	{
		if(args.length != 3)
		{
			console.log("Usage: fastevm transferto <to> <amount>")
			console.log("Example: fastevm transferto tarkeytarkey 123")
			process.exit(0);
		}
		var fid = str2name(args[1])
		var buff = Buffer.from(BigInt(fid).toString(16), 'hex');
		var target = buff.reverse().toString('hex')
		var output = '0xa9059cbb000000000000000000000000000000000000000000000000' + target + to256hex(args[2])
		console.log(output)
		break;
	}
}


// console.log('fastid = ', fid)