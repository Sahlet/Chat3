using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Net;
using System.Net.Sockets;

namespace chat_client
{
    public partial class Form2 : Form{
        public bool aut;
        private Socket sock;
        string[] commands, answers;
        string key;
        public Form2(string Title, string label1, string label2, string[] commands, string[] answers, string key)
        {
            InitializeComponent();
            setTitle(Title);
            setLabel1(label1);
            setLabel2(label2);
            aut = false;
            sock = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
            Central.connect(sock);
            this.commands = commands;
            this.answers = answers;
            this.key = key;
        }

        private void button1_Click(object sender, EventArgs e){
            Central.write(commands[0], sock);
            Central.write(getParam1(), sock);
            Central.write(getParam2(), sock);
            Central.write(key, sock);
            string res = Central.read(sock);
            if (res == answers[0]){
                sock.Close();
                aut = true;
                this.Close();
            } else MessageBox.Show(Central.read(sock), "Message from server.");
        }
        private void button2_Click(object sender, EventArgs e){
            if (getParam1() == ""){
                MessageBox.Show("User cannot have empty login.", "Warning.");
                return;
            }
            Central.write(commands[1], sock);
            Central.write(getParam1(), sock);
            Central.write(getParam2(), sock);
            string res = Central.read(sock);
            if (res == answers[1]) MessageBox.Show(Central.read(sock), "Message from server.");
            else MessageBox.Show("user with login: \' " + getParam1() + " \' registrated.", "Message from server.");
        }
        public string getParam1() { return textBox1.Text; }
        public string getParam2() { return textBox2.Text; }

        public void setTitle(string s) { this.Text = s; }
        public void setLabel1(string s) { label1.Text = s; }
        public void setLabel2(string s) { label2.Text = s; }
    }
}
