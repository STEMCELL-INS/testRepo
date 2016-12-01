using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Runtime.InteropServices;

namespace EasyPickTestGUI
{
    public partial class Form1 : Form
    {
        [DllImport("C:\\projects\\EasyPickFL_ImgProcessing\\EasyPickDLL\\Release\\EasyPickDll.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int process_day4(string folderPath);

        public Form1()
        {
            InitializeComponent();
        }

        private void button1_Click(object sender, EventArgs e)
        {
            try
            {
                string path = "C:\\projects\\EasyPickFL_ImgProcessing\\easyPick Images\\";
                process_day4(path);
            }
            catch (Exception err)
            {
                string temp = err.Message;
            }
        }

        private void button2_Click(object sender, EventArgs e)
        {
            Application.Exit();
        }

    }
}
