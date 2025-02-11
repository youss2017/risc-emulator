#include <iostream>
#include <cstdint>
#include <string>
#include <optional>
#include <vector>
#include <format>
#include <map>
#include <fstream>
#include <sstream>
using namespace std;

struct ArgOpt {
	bool MustBeSupplied = false;
	bool CaseSensitive = false;
	string DefaultValue;

	ArgOpt() = default;
	ArgOpt(bool mustBeSupplied) : MustBeSupplied(mustBeSupplied) {}
	ArgOpt(const string& defaultValue) : DefaultValue(defaultValue) {}
	ArgOpt(bool mustBeSupplied, bool caseSensitive) : MustBeSupplied(mustBeSupplied), CaseSensitive(caseSensitive) {}
	ArgOpt(bool mustBeSupplied, const string& defaultValue) : MustBeSupplied(mustBeSupplied), DefaultValue(defaultValue) {}
	ArgOpt(bool mustBeSupplied, bool caseSensitive, const string& defaultValue) : MustBeSupplied(mustBeSupplied), CaseSensitive(caseSensitive), DefaultValue(defaultValue) {}

};

struct ArgResult {
	map<string, string> ArgToValue;
	vector<string> MissingArguments;
};

ArgResult ProccessArguments(int argc, char** argv,
	const map<string, ArgOpt>& programArgumentList) {

	ArgResult result;

	for (auto& [argName, opt] : programArgumentList) {
		if (!opt.DefaultValue.empty() && !opt.MustBeSupplied)
			result.ArgToValue[argName] = opt.DefaultValue;
		if (opt.MustBeSupplied)
			result.MissingArguments.push_back(argName);
	}

	bool isValue = false;
	string valueArgName;
	for (int i = 0; i < argc; i++) {
		if (isValue) {
			isValue = false;
			result.ArgToValue[valueArgName] = argv[i];
			auto it = find(result.MissingArguments.begin(), result.MissingArguments.end(), valueArgName);
			if (it != result.MissingArguments.end())
				result.MissingArguments.erase(it);
			continue;
		}
		for (auto& [argName, opt] : programArgumentList) {

			bool stringCmpResult = false;
			if (opt.CaseSensitive) {
				stringCmpResult = argv[i] == argName;
			}
			else {
				string lowerCaseArgName = argName;
				string lowerCaseArgV = argv[i];
				for (int i = 0; i < lowerCaseArgName.length(); i++)
					lowerCaseArgName[i] = tolower(lowerCaseArgName[i]);
				for (int i = 0; i < lowerCaseArgV.length(); i++)
					lowerCaseArgV[i] = tolower(lowerCaseArgV[i]);
				stringCmpResult = argv[i] == argName;
			}

			if (!stringCmpResult)
				continue;

			valueArgName = argName;
			isValue = true;
		}
	}

	return result;
}

ifstream OpenFileStream(const string& path) {
	ifstream file(path);
	if (!file) {
		cerr << "\tERROR: Failed to load file '" << path << "'";
		exit(0);
	}
	return file;
}

uint32_t ParseRegister(const string& regText, int line) {
	string regClean;
	for (int i = 0; i < regText.length(); i++) {
		if (regText[i] == ' ' || regText[i] == ',') continue;
		regClean.push_back(regText[i]);
	}
	if (regClean == "x0") return 0;
	if (regClean == "x1") return 1;
	if (regClean == "x2") return 2;
	if (regClean == "x3") return 3;
	if (regClean == "x4") return 4;
	if (regClean == "x5") return 5;
	if (regClean == "x6") return 6;
	if (regClean == "x7") return 7;
	if (regClean == "x8") return 8;
	if (regClean == "x9") return 9;
	if (regClean == "x10") return 10;
	if (regClean == "x11") return 11;
	if (regClean == "x12") return 12;
	if (regClean == "x13") return 13;
	if (regClean == "x14") return 14;
	if (regClean == "x15") return 15;
	if (regClean == "x16") return 16;
	if (regClean == "x17") return 17;
	if (regClean == "x18") return 18;
	if (regClean == "x19") return 19;
	if (regClean == "x20") return 20;
	if (regClean == "x21") return 21;
	if (regClean == "x22") return 22;
	if (regClean == "x23") return 23;
	if (regClean == "x24") return 24;
	if (regClean == "x25") return 25;
	if (regClean == "x26") return 26;
	if (regClean == "x27") return 27;
	if (regClean == "x28") return 28;
	if (regClean == "x29") return 29;
	if (regClean == "x30") return 30;
	if (regClean == "x31") return 31;
	cerr << "ERROR: Failed to parse register number from " << regText << " at line: " << line << "\n\n";
	exit(0);
	return -1;
}

uint32_t GetInstructionFunct3(const string& instructionName, int line) {
	if (instructionName == "lui") return 0b000;
	if (instructionName == "auipc") return 0b000;
	if (instructionName == "jal") return 0b000;
	if (instructionName == "jalr") return 0b000;
	if (instructionName == "beq") return 0b000;
	if (instructionName == "bne") return 0b001;
	if (instructionName == "blt") return 0b100;
	if (instructionName == "bge") return 0b101;
	if (instructionName == "bltu") return 0b110;
	if (instructionName == "bgeu") return 0b111;
	if (instructionName == "lb") return 0b000;
	if (instructionName == "lh") return 0b001;
	if (instructionName == "lw") return 0b010;
	if (instructionName == "lbu") return 0b100;
	if (instructionName == "lhu") return 0b101;
	if (instructionName == "sb") return 0b000;
	if (instructionName == "sh") return 0b001;
	if (instructionName == "sw") return 0b010;
	if (instructionName == "addi") return 0b000;
	if (instructionName == "slti") return 0b010;
	if (instructionName == "sltiu") return 0b011;
	if (instructionName == "xori") return 0b100;
	if (instructionName == "ori") return 0b110;
	if (instructionName == "andi") return 0b111;
	if (instructionName == "slli") return 0b001;
	if (instructionName == "srli") return 0b101;
	if (instructionName == "srai") return 0b101;
	if (instructionName == "add") return 0b000;
	if (instructionName == "sub") return 0b000;
	if (instructionName == "sll") return 0b001;
	if (instructionName == "slt") return 0b010;
	if (instructionName == "sltu") return 0b011;
	if (instructionName == "xor") return 0b100;
	if (instructionName == "srl") return 0b101;
	if (instructionName == "sra") return 0b101;
	if (instructionName == "or") return 0b110;
	if (instructionName == "and") return 0b111;
	cerr << "ERROR: Unknown opcode encountered '" << instructionName << "' at line " << line << "\n\n";
	exit(0);
	return -1;
}

uint32_t GetInstructionFunct7(const string& instructionName, int line) {
	if (instructionName == "lui") return 0;
	if (instructionName == "auipc") return 0;
	if (instructionName == "jal") return 0;
	if (instructionName == "jalr") return 0;
	if (instructionName == "beq") return 0;
	if (instructionName == "bne") return 0;
	if (instructionName == "blt") return 0;
	if (instructionName == "bge") return 0;
	if (instructionName == "bltu") return 0;
	if (instructionName == "bgeu") return 0;
	if (instructionName == "lb") return 0;
	if (instructionName == "lh") return 0;
	if (instructionName == "lw") return 0;
	if (instructionName == "lbu") return 0;
	if (instructionName == "lhu") return 0;
	if (instructionName == "sb") return 0;
	if (instructionName == "sh") return 0;
	if (instructionName == "sw") return 0;
	if (instructionName == "addi") return 0;
	if (instructionName == "slti") return 0;
	if (instructionName == "sltiu") return 0;
	if (instructionName == "xori") return 0;
	if (instructionName == "ori") return 0;
	if (instructionName == "andi") return 0;
	if (instructionName == "slli") return 0b0000000;
	if (instructionName == "srli") return 0b0000000;
	if (instructionName == "srai") return 0b0100000;
	if (instructionName == "add") return 0b0000000;
	if (instructionName == "sub") return 0b0100000;
	if (instructionName == "sll") return 0b0000000;
	if (instructionName == "slt") return 0b0000000;
	if (instructionName == "sltu") return 0b0000000;
	if (instructionName == "xor") return 0b0000000;
	if (instructionName == "srl") return 0b0000000;
	if (instructionName == "sra") return 0b0100000;
	if (instructionName == "or") return 0b0000000;
	if (instructionName == "and") return 0b0000000;
	cerr << "ERROR: Unknown opcode encountered '" << instructionName << "' at line " << line << "\n\n";
	exit(0);
	return -1;
}

uint32_t GetOpCode(const string& instructionName, int line) {
	if (instructionName == "lui") return 0b0110111;
	if (instructionName == "auipc") return 0b0010111;
	if (instructionName == "jal") return 0b1101111;
	if (instructionName == "jalr") return 0b1100111;
	if (instructionName == "beq") return 0b1100011;
	if (instructionName == "bne") return 0b1100011;
	if (instructionName == "blt") return 0b1100011;
	if (instructionName == "bge") return 0b1100011;
	if (instructionName == "bltu") return 0b1100011;
	if (instructionName == "bgeu") return 0b1100011;
	if (instructionName == "lb") return 0b0000011;
	if (instructionName == "lh") return 0b0000011;
	if (instructionName == "lw") return 0b0000011;
	if (instructionName == "lbu") return 0b0000011;
	if (instructionName == "lhu") return 0b0000011;
	if (instructionName == "sb") return 0b0100011;
	if (instructionName == "sh") return 0b0100011;
	if (instructionName == "sw") return 0b0100011;
	if (instructionName == "addi") return 0b0010011;
	if (instructionName == "slti") return 0b0010011;
	if (instructionName == "sltiu") return 0b0010011;
	if (instructionName == "xori") return 0b0010011;
	if (instructionName == "ori") return 0b0010011;
	if (instructionName == "andi") return 0b0010011;
	if (instructionName == "slli") return 0b0010011;
	if (instructionName == "srli") return 0b0010011;
	if (instructionName == "srai") return 0b0010011;
	if (instructionName == "add") return 0b0110011;
	if (instructionName == "sub") return 0b0110011;
	if (instructionName == "sll") return 0b0110011;
	if (instructionName == "slt") return 0b0110011;
	if (instructionName == "sltu") return 0b0110011;
	if (instructionName == "xor") return 0b0110011;
	if (instructionName == "srl") return 0b0110011;
	if (instructionName == "sra") return 0b0110011;
	if (instructionName == "or") return 0b0110011;
	if (instructionName == "and") return 0b0110011;
	cerr << "ERROR: Unknown opcode encountered '" << instructionName << "' at line " << line << "\n\n";
	exit(0);
	return -1;
}

string GetInstructionType(const string& instructionName, int line) {
	if (instructionName == "lui") return "u";
	if (instructionName == "auipc") return "u";
	if (instructionName == "jal") return "j";
	if (instructionName == "jalr") return "i";
	if (instructionName == "beq") return "b";
	if (instructionName == "bne") return "b";
	if (instructionName == "blt") return "b";
	if (instructionName == "bge") return "b";
	if (instructionName == "bltu") return "b";
	if (instructionName == "bgeu") return "b";
	if (instructionName == "lb") return "i";
	if (instructionName == "lh") return "i";
	if (instructionName == "lw") return "i";
	if (instructionName == "lbu") return "i";
	if (instructionName == "lhu") return "i";
	if (instructionName == "sb") return "s";
	if (instructionName == "sh") return "s";
	if (instructionName == "sw") return "s";
	if (instructionName == "addi") return "i";
	if (instructionName == "slti") return "i";
	if (instructionName == "sltiu") return "i";
	if (instructionName == "xori") return "i";
	if (instructionName == "ori") return "i";
	if (instructionName == "andi") return "i";
	if (instructionName == "slli") return "i";
	if (instructionName == "srli") return "i";
	if (instructionName == "srai") return "i";
	if (instructionName == "add") return "r";
	if (instructionName == "sub") return "r";
	if (instructionName == "sll") return "r";
	if (instructionName == "slt") return "r";
	if (instructionName == "sltu") return "r";
	if (instructionName == "xor") return "r";
	if (instructionName == "srl") return "r";
	if (instructionName == "sra") return "r";
	if (instructionName == "or") return "r";
	if (instructionName == "and") return "r";
	cerr << "ERROR: Unknown opcode encountered '" << instructionName << "' at line " << line << "\n\n";
	exit(0);
	return "";
}

uint32_t ParseImmediateValue(string token, int line, uint32_t* pOutRs1 = nullptr) {
	auto open = token.find("(");
	auto close = token.find(")");
	if (open == size_t(-1) && close == size_t(-1)) {
		try {
			return stoi(token);
		}
		catch (...) {
			cerr << "ERROR: Could not parse immediate value '" << token  << "' at line: " << line << "\n\n";
			exit(0);
		}
	}
	if (open == size_t(-1) || close == size_t(-1) || open == close) {
		cerr << "ERROR: Invalid syntax '" << token << "' at line: " << line << "\n\n";
		exit(0);
	}
	if (open == 0) {
		token = "0" + token;
	}
	string imm_text = token.substr(0, open);
	string rs1 = token.substr(open + 1, (close-open)-1);
	uint32_t imm = 0;

	if (pOutRs1) {
		*pOutRs1 = ParseRegister(rs1, line);
	}

	try {
		return stoi(imm_text);
	}
	catch (...) {
		cerr << "ERROR: Could not parse immediate value '" << token << "' at line: " << line << "\n\n";
		exit(0);
	}
	return -1;
}

uint32_t MakeRTypeInstruction(const vector<string>& tokens, int line, uint32_t* pRd, uint32_t* pRs1, uint32_t* pRs2) {
	auto opcode = GetOpCode(tokens[0], line);
	auto funct3 = GetInstructionFunct3(tokens[0], line);
	auto funct7 = GetInstructionFunct7(tokens[0], line);
	auto rd = ParseRegister(tokens[1], line);
	auto rs1 = ParseRegister(tokens[2], line);
	auto rs2 = ParseRegister(tokens[3], line);
	if (pRd)
		*pRd = rd;
	if (pRs1) *pRs1 = rs1;
	if (pRs2) *pRs2 = rs2;
	// secret sause
	return (funct7 << 25) | (rs2 << 20) | (rs1 << 15) | (funct3 << 12) | (rd << 7) | (opcode);
}

uint32_t MakeITypeInstruction(const vector<string>& tokens, int line, uint32_t* pRd, uint32_t* pRs1) {
	auto opcode = GetOpCode(tokens[0], line);
	auto funct3 = GetInstructionFunct3(tokens[0], line);
	auto funct7 = GetInstructionFunct7(tokens[0], line);
	auto rd = ParseRegister(tokens[1], line);
	auto rs1 = ParseRegister(tokens[2], line);
	int32_t imm = ParseImmediateValue(tokens[3], line);
	if (pRd) *pRd = rd;
	if (pRs1) *pRs1 = rs1;
	// secret sauce
	return (imm << 20) | (rs1 << 15) | (funct3 << 12) | (rd << 7) | (opcode);
}

uint32_t MakeBTypeInstruction(const vector<string>& tokens, int line, uint32_t* pRs1, uint32_t* pRs2) {
	auto opcode = GetOpCode(tokens[0], line);
	auto funct3 = GetInstructionFunct3(tokens[0], line);
	auto funct7 = GetInstructionFunct7(tokens[0], line);
	auto rs1 = ParseRegister(tokens[1], line);
	auto rs2 = ParseRegister(tokens[2], line);
	int32_t imm = ParseImmediateValue(tokens[3], line);
	int32_t upper_imm = (imm & 0b111111100000) >> 5;
	int32_t lower_imm = imm & 0b000000011111;
	if (pRs1) *pRs1 = rs1;
	if (pRs2) *pRs2 = rs2;
	// secret sauce
	return (upper_imm << 25) | (rs2 << 20) | (rs1 << 15) | (funct3 << 12) | (lower_imm << 7) | (opcode);
}

uint32_t MakeSTypeInstruction(const vector<string>& tokens, int line, uint32_t* pRs1) {
	auto opcode = GetOpCode(tokens[0], line);
	auto funct3 = GetInstructionFunct3(tokens[0], line);
	auto rs2 = ParseRegister(tokens[1], line);
	uint32_t rs1 = 0;
	int32_t imm = ParseImmediateValue(tokens[2], line, &rs1);
	int32_t upper_imm = (imm & 0b111111100000) >> 5;
	int32_t lower_imm = imm & 0b000000011111;
	if (pRs1) *pRs1 = rs1;

	// secret sauce
	return (upper_imm << 25) | (rs2 << 20) | (rs1 << 15) | (funct3 << 12) | (lower_imm << 7) | (opcode);
}

uint32_t MakeITypeInstruction3(const vector<string>& tokens, int line, uint32_t* pRd, uint32_t* pRs1) {
	auto opcode = GetOpCode(tokens[0], line);
	auto funct3 = GetInstructionFunct3(tokens[0], line);
	auto funct7 = GetInstructionFunct7(tokens[0], line);
	auto rd = ParseRegister(tokens[1], line);
	uint32_t rs1;
	int32_t imm = ParseImmediateValue(tokens[2], line, &rs1);
	if (pRd) *pRd = rd;
	if (pRs1) *pRs1 = rs1;
	// secret sauce
	return (imm << 20) | (rs1 << 15) | (funct3 << 12) | (rd << 7) | (opcode);
}

uint32_t CompileInstruction(const vector<string>& tokens, int line, uint32_t* pOutReg, uint32_t* pOutRs1, uint32_t* pOutRs2) {
	uint32_t THE_CODE = 0;

	auto AssertNTokens = [&](int n) {
		if (tokens.size() < n) {
			cerr << "\tERROR: Compiler does not support multi-line instructions. Check line " << line << endl;
			exit(0);
		}
	};

	if (tokens.size() == 4) {
		auto type = GetInstructionType(tokens[0], line);
		if (type == "r")
			return MakeRTypeInstruction(tokens, line, pOutReg, pOutRs1, pOutRs2);
		else if (type == "i")
			return MakeITypeInstruction(tokens, line, pOutReg, pOutRs1);
		else if (type == "b")
			return MakeBTypeInstruction(tokens, line, pOutRs1, pOutRs2);
	}
	else if (tokens.size() == 3) {
		auto type = GetInstructionType(tokens[0], line);
		if (type == "s")
			return MakeSTypeInstruction(tokens, line, pOutRs1);
		else if (type == "i")
			return MakeITypeInstruction3(tokens, line, pOutReg, pOutRs1);
	}

	cerr << "\tERROR: Unsupported Condition unsupported, youssef fix this.\n";
	exit(0);

	return THE_CODE;
}

vector<uint32_t> CompileFile(ifstream& input) {
	auto result = vector<uint32_t>{};

	string line;
	map<string, uint32_t> labels;
	int lineNumber = 1;

	uint32_t deps[2]{};

	while (std::getline(input, line)) {
		istringstream iss(line);
		string word;
		vector<string> tokens;
		cout << "\tprocesssing " << line << endl;
		// split by whitespace
		while (iss >> word) {
			string token;
			for (int i = 0; i < word.length(); i++) if(word[i] != ',') token.push_back(tolower(word[i]));
			if (token == "#" || token == "//") break;
			if (token == "/*") {
				cerr << "\tERROR: Multiline comments are not supported at line: " << line << endl;
				exit(0);
			}
			tokens.push_back(token);
		}
		// Is this a label?
		if (tokens.size() == 1 && tokens[0].ends_with(":")) {
			string label = tokens[0].substr(0, tokens[0].find(":") - 1);
			if (labels.contains(label)) {
				cerr << "\tERROR: Duplicate labels at line: " << line << endl;
				exit(0);
			}
		}
		uint32_t writeReg{};
		uint32_t rs1{};
		uint32_t rs2{};
		auto instruction = CompileInstruction(tokens, lineNumber, &writeReg, &rs1, &rs2);
		if ((rs1 == deps[0] || rs2 == deps[0]) && (rs1 || rs2)) {
			// add 2 nops
			result.push_back(0);
			result.push_back(0);
		}
		else if ((rs1 == deps[1] || rs2 == deps[1]) && writeReg && (rs1 || rs2)) {
			// add 1 dep
			result.push_back(0);
		}
		deps[1] = deps[0];
		deps[0] = writeReg;
		result.push_back(instruction);
		lineNumber++;
	}

	return result;
}

int main(int argc, char** argv) {

	auto cargs = ProccessArguments(argc, argv, {
		make_pair("-i", ArgOpt{true}),
		make_pair("-o", ArgOpt{string("a.bin")}),
	});

	auto file = cargs.ArgToValue["-i"];
#ifdef _DEBUG
	 if (file.empty()) file = "hello.asm";
#endif
	cout << "Compiling " << file << "...\n";

	auto ifs = OpenFileStream(file);
	auto code = CompileFile(ifs);

	return 0;
}