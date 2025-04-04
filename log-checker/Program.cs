using System.IO;
using System.Text;

namespace log_checker;

class Program
{
    public struct RegFile
    {
        public uint instruction;
        public readonly uint[] state;
        public RegFile() => state = new uint[32];
    };

    static void Main(string[] args)
    {
        Console.OutputEncoding = Encoding.UTF8;

        var sim = File.ReadAllText("C:\\Users\\youssef\\source\\repos\\risc-emulator\\risc-emulator\\emulator.log");
        var core = File.ReadAllText("U:\\Senior Design\\RAPID-X Core\\vivado-project\\rapid-x\\rapid-x.sim\\sim_1\\behav\\xsim\\core.log");

        var simState = ParseLog(sim);
        var coreState = ParseLog(core);
        SaveAsCSV("emulator-csv.csv", simState.Take(1000).ToList(), false);
        SaveAsCSV("core-csv.csv", coreState.Take(1000).ToList(), true);

        for (int i = 0; i < Math.Min(simState.Count, coreState.Count-1); i++)
        {
            for (int j = 0; j < 32; j++)
            {
                if (simState[i].state[j] != coreState[i+1].state[j])
                {
                    Console.ForegroundColor = ConsoleColor.Red;
                    Console.WriteLine($"{i} :( --- register {j} mismatched.");
                    Console.ForegroundColor = ConsoleColor.Yellow;
#if RELEASE
                    Console.WriteLine("RELEASE MODE!");
#else
                    Console.WriteLine("DEBUG MODE!");
#endif
                    Environment.Exit(100);
                }
            }
        Console.ForegroundColor = ConsoleColor.Green;
            Console.WriteLine($"{i} :)");
        }

        Console.ForegroundColor = ConsoleColor.Green;
        Console.WriteLine("Good Job! 100% Equivalence");

        Environment.Exit(0xea);

    }

    static void SaveAsCSV(string fileName, List<RegFile> data, bool isCore)
    {
        StreamWriter stream = new StreamWriter(new FileStream(fileName, FileMode.Create));
        stream.Write("instr,");
        for (int i = 0; i < 32; i++)
        {
            stream.Write($"x{i},");
        }
        stream.Write('\n');
        string prevInstruction = "";
        foreach (var state in data)
        {
            if (!string.IsNullOrWhiteSpace(prevInstruction) || !isCore)
            {
                stream.Write(isCore ? prevInstruction : state.instruction.ToString("x8"));
                stream.Write(',');
                foreach (var reg in state.state)
                {
                    stream.Write(reg.ToString("x"));
                    stream.Write(',');
                }
                stream.Write('\n');
            }
            prevInstruction = state.instruction.ToString("x8");
        }
        stream.Close();
    }


    static void SaveAsXLS(string fileName, List<RegFile> data, bool isCore)
    {
        StreamWriter stream = new StreamWriter(new FileStream(fileName, FileMode.Create));
        stream.Write("<table><thead><tr>");
        stream.Write("<th>instr</th>");
        for (int i = 0; i < 32; i++)
        {
            stream.Write($"<th>x{i}</th>");
        }
        stream.Write("</thead><tbody>");
        string prevInstruction = "";
        RegFile prevState = new RegFile();
        foreach (var state in data)
        {
            if (!string.IsNullOrWhiteSpace(prevInstruction) || !isCore)
            {
                stream.Write("<tr><td>");
                stream.Write(isCore ? prevInstruction : state.instruction.ToString("x8"));
                stream.Write("</td>");
                int index = 0;
                foreach (var reg in state.state)
                {
                    if (reg != prevState.state[index])
                    {
                        stream.Write($"<td style='color: red;font-weight: bold'>{reg.ToString("x")}</td>");
                    }
                    else
                    {
                        stream.Write($"<td></td>");
                    }
                    index++;
                }
                stream.Write("</tr>");
            }
            prevState = state;
            prevInstruction = state.instruction.ToString("x8");
        }
        stream.Close();
    }


    static List<RegFile> FilterLog(List<RegFile> log)
    {
        List<RegFile> result = new();
        if (log.Count == 0) return result;
        result.Add(log[0]);
        for (int i = 1; i < log.Count; i += 2)
        {
            if (!CompareState(result[result.Count - 1], log[i]))
                result.Add(log[i]);
        }
        return result;
    }

    /// <summary>
    /// Compares two register states.
    /// </summary>
    /// <returns>Returns two if both states are equal</returns>
    static bool CompareState(RegFile stateA, RegFile stateB)
    {
        for (int j = 0; j < 32; j++)
        {
            if (stateA.state[j] != stateB.state[j])
            {
                return false;
            }
        }
        return true;
    }

    static List<RegFile> ParseLog(string logText)
    {
        List<RegFile> simResult = new();
        foreach (var line in logText.Split("\n"))
        {
            if (string.IsNullOrWhiteSpace(line)) continue;

            int i = 0;
            RegFile f = new();

            var start = line.IndexOf('(') + 1;
            f.instruction = Convert.ToUInt32(line[start..line.IndexOf(')')], 16);

            if (f.instruction == 0x00000033 /* nop */) continue;

            var dummy = line.Substring(line.IndexOf(":") + 1).Trim();
            var dummy2 = dummy.Split(" ", StringSplitOptions.RemoveEmptyEntries | StringSplitOptions.TrimEntries);
            foreach (var number in dummy2)
            {
                f.state[i++] = Convert.ToUInt32(number.Replace("x", "0").Replace("X", "0").Trim(), 16);
            }
            simResult.Add(f);
        }
        return simResult;
    }

}
