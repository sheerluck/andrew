using System;
using System.Collections.Generic;
					
public class Program
{
    public static void Main()
    {
        var t1 = new List<string> { "123", "Jack", "Jill", "777" };
        var t2 = new List<string> { "123", "Jack", "Noll", "777" };
        var t3 = new List<string> { "123", "Nock", "Noll", "777" };
        var t4 = new List<string>();

        var tests =  new List<List<string>> {t1, t2, t3, t4 };
       
        foreach (List<string> test in tests)
        {
            var names = test;

            Func<bool> moreNames = () => names.Count > 0;
            Func<string> getName = delegate()
            {
                var last = names.ToArray().GetValue(names.Count - 1);
                names.RemoveAt(names.Count - 1);
                return last as string;
            };

            Func<string> name =  delegate()
            {
                var last = "^last^";
                while (moreNames()) {
                    var name_ = getName();
                    if (name_ == "Jack" || name_ == "Jill") return name_;
                    last = name_;
                }
                if (last == "^last^") return "Herbert";
                return last;
            };

            Console.WriteLine("Hello, " + name());
        }
    }
}
