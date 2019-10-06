class permutation
{
	constructor(set, head=[], all=[])
	{
		this.all = all
		this.i = 0
		for(let a of set)
		{
			let neck = head.concat(a)
			if(set.length == 1)
				all.push(neck)
			else
				new permutation(set.filter(x=>x!=a), neck, all)
		}
	}
	next()
	{
		if(this.i < this.all.length)
			return this.all[this.i++]
		return false
	}
}

class code
{
	constructor(n, m)
	{
		this.n = n
		this.m = m
		this.digits = new Array(m).fill(0)
		this.new = true
	}
	next()
	{
		if(this.new)
		{
			this.new = false
			return this.digits
		}
		let j = this.m-1
		while(j>=0 && this.digits[j]==this.n-1)
			j--
		if(j<0) return false
		this.digits[j]++
		if(j==this.m-1) return this.digits
		for(j++;j<this.m;j++)
			this.digits[j]=0
		return this.digits
	}
}

class ticket
{
	constructor(digitStr)
	{
		this.digits = digitStr.split('')
		this.n = this.digits.length
		this.opSymbols = ['', '+', '-', '*']
		this.answers = new Set()
		this.loops = 0
	}
	getAnswers(sum)
	{
		let opCode = new code(this.opSymbols.length, this.n-1)
		let operations
		while(operations = opCode.next())
		{
			let skip = false
			let i = 0, notEmptyOps = 0;
			let str = this.digits[i++];
			let opInStr = []
			for(let op of operations)
			{
				let lst = str.length-1
				if(op != 0) opInStr[notEmptyOps++] = lst+1
				else if((lst==0||this.opSymbols.includes(str[lst-1]))&&str[lst]=='0')
					skip = true
				str += this.opSymbols[op]
				str += this.digits[i++]
			}
			if(skip) continue
			
			let orders = new permutation([...Array(notEmptyOps).keys()])
			let order
			while(order = orders.next())
			{
				this.loops++
				let brackets = new Array(str.length).fill(null).map(()=>{return{l:'',r:''}})
				while(order.length>1)
				{
					let curOp = opInStr[order.shift()]
					let leftSum = 0, left = curOp, leftInt = false
					do
					{
						leftInt = false
						left--
						leftSum += brackets[left].r.length
						leftSum -= brackets[left].l.length
						if(left<1 || this.opSymbols.includes(str[left-1]))
							leftInt = true
					}while(leftSum > 0 || !leftInt)
					brackets[left].l += '('
					let rightSum = 0, right = curOp, rightInt = false
					do
					{
						rightInt = false
						right++
						rightSum += brackets[right].l.length
						rightSum -= brackets[right].r.length
						if(right+1==str.length || this.opSymbols.includes(str[right+1]))
							rightInt = !rightInt
					}while(rightSum > 0 || !rightInt)
					brackets[right].r += ')'

				}
				let braceStr = ''
				for(let i=0; i<str.length; ++i)
				{
					braceStr += brackets[i].l
					braceStr += str[i]
					braceStr += brackets[i].r
				}
				let curSum = eval(braceStr)
				if(curSum == sum)
				{
					this.answers.add(braceStr)
				}
				
			}
		}
		return this.answers;
	}
}


let myLuckyTicket = new ticket(process.argv[2])
let answers = myLuckyTicket.getAnswers(100)
console.log(answers)