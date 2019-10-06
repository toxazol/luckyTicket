#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <climits>
using namespace std;

struct code
{
	code(int N, int M)
	: n(N), m(M), digits(M, 0), newCode(true)
	{}
	vector<int> next()
	{
		if(newCode)
		{
			newCode = false;
			return digits;
		}
		int j = m-1;
		while(j>=0 && digits[j]==n-1)
			j--;
		if(j<0) return {};
		digits[j]++;
		if(j==m-1) return digits;
		for(j++;j<m;j++)
			digits[j]=0;
		return digits;
	}
private:
	int n, m;
	vector<int> digits;
	bool newCode;
};
struct ticket
{
	ticket(string digitString)
	{
		m_digitStr = digitString;
		m_binaryOp = 6; // NULL, '+', '-', '*', '/', '^'
		//m_unaryOp = 3; // NULL, '!', '-'
		loops = 0;
	}
	bool isLucky(int sum)
	{
		auto opCode = new code(m_binaryOp, m_digitStr.length()-1);
		int count = 0;
		while(1)
		{
			bool skip = false;
			auto operations = opCode->next();
			if(operations.empty()) break;
			vector<int> operands;
			for(int i=0, k=1; true; k++)
			{
				if(i+k==m_digitStr.length())
				{
					string cur = m_digitStr.substr(i,k);
					if(cur[0] == '0')
					{
						skip = true;
						break;
					} 
					operands.push_back(stoi(cur));
					break;
				}
				if(operations[i+k-1])
				{
					string cur = m_digitStr.substr(i,k);
					if(cur.length() > 1 && cur[0] == '0')
					{
						skip = true;
						break;
					} 
					operands.push_back(stoi(cur));
					i = i+k; k = 0;
				}
			}
			if(skip) continue;
			operations.erase(remove(operations.begin(), operations.end(), 0), operations.end());
			vector<int> order(operations.size());
			iota(order.begin(),order.end(),0);
			do
			{	
				loops++;
				vector<pair<int,int> > opStack;
				for(auto a: order)
					opStack.push_back(make_pair(a,a));

				vector<int> curOperands(operands);
				while(!opStack.empty())
				{
					pair<int,int> curOp = opStack.back(); opStack.pop_back();
					bool err = false;
					switch(operations[curOp.second])
					{
						case 1:	curOperands[curOp.first] += curOperands[curOp.first+1];
								break;
						case 2: curOperands[curOp.first] -= curOperands[curOp.first+1];
								break;
						case 3: curOperands[curOp.first] *= curOperands[curOp.first+1];
								break;
						case 4: if(curOperands[curOp.first+1]==0 ||
									curOperands[curOp.first]%curOperands[curOp.first+1]!=0)
									err = true;
								else
									curOperands[curOp.first] = curOperands[curOp.first+1];
								break;
						case 5:	if(curOperands[curOp.first] == 0)
								{
									curOperands[curOp.first] = 0;
									break;
								}
								double powRes = pow(curOperands[curOp.first], curOperands[curOp.first+1]);
								if(powRes > INT_MAX)
									err = true;
								else
									curOperands[curOp.first] = int(powRes);
								break;
					}
					if(err) {curOperands[0] = sum+1; break;}
					curOperands.erase(curOperands.begin()+curOp.first+1);
					for(int i=0; i<opStack.size(); i++)
						if(opStack[i].first>curOp.first) opStack[i].first--;
				}
				if(curOperands[0] == sum)
					return true;
				else continue;
				
			}while(next_permutation(order.begin(), order.end()));
		}
		return false;
	}

	int loops;
private:
	string m_digitStr;
	size_t m_binaryOp;
	//size_t m_unaryOp;
};

int main(int argc, char *argv[])
{
	const int ticketLength = 6, luckySum = 100;

	auto billion = new code(10, ticketLength);
	vector<string> codes;
	while(1)
	{
		auto nextCode = billion->next();
		if(nextCode.empty()) break;
		for(int& c: nextCode) c+= '0';
		codes.push_back(string(nextCode.begin(), nextCode.end()));
	}

	size_t luckyTotal = 0, stepsCompleted = 0; 
	unsigned long long int loopsTotal = 0;
	auto start = chrono::system_clock::now();
	auto stepSize = 100ul; auto stepsTotal = codes.size()/stepSize + 1;
	vector<string> unlucky;

#pragma opm parallel
{
	size_t localTotal = 0;
#pragma omp parallel for reduction(+:luckyTotal,loopsTotal)
	for(size_t i=0;i<codes.size(); ++i)
	{	
		auto tick = new ticket(codes[i]);
		bool isLuck = tick->isLucky(luckySum);
		luckyTotal += isLuck;
		loopsTotal += tick->loops;
		if(!isLuck)
#pragma omp critical	
			unlucky.push_back(codes[i]);
		if(localTotal++%stepSize==stepSize-1)
		{
#pragma omp atomic
			stepsCompleted++;
			if(stepsCompleted % 10 == 1)
			{
#pragma omp critical
				cout << fixed << setprecision(1) 
				<< 100.0*stepsCompleted/stepsTotal << "% of tickets are processed" << endl;
			}
		}
	}
}
	auto end = chrono::system_clock::now();
	auto hours = std::chrono::duration_cast<std::chrono::hours>(end - start).count();
	auto minutes = std::chrono::duration_cast<std::chrono::minutes>(end - start).count()%60;
	auto seconds = std::chrono::duration_cast<std::chrono::seconds>(end - start).count()%60;

	ofstream file_id;
	file_id.open("unlucky_extended.out");
	for(auto t: unlucky)
		file_id << t << endl;
	file_id.close();

	cout << luckyTotal << " tickets out of " << codes.size() << " are lucky." << endl;
	cout << "Luck probability is " << (100.0*luckyTotal)/codes.size() << "%." << endl;
	cout << loopsTotal << " expressions processed in " 
	<< hours << "h " << minutes << "m " << seconds << "s " << endl;
	cout << unlucky.size() << " unlucky tickets listed in /unlucky_extended.out" << endl;
}