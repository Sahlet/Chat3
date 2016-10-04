using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace chat_client
{
    public partial class AdderFriend : Form
    {
        User u;
        public AdderFriend(User u)
        {
            InitializeComponent();
            this.u = u.copy();
            this.Controls.Add(this.u);
        }

        void add() {
            Waiter w = new Waiter(ref Central.counter[(int)Central.key.sender_request, 0], ref Central.counter[(int)Central.key.sender_request, 1], new method_empty(this.add_));
        }
        void add_(){
            int k = (int)Central.key.sender_request;
            if (Central.socks[k] == null) Central.set_right(k);
            if (Central.socks[k] == null) Central.Fatal_Error();
            Central.write(u.Name, Central.socks[k]);
            Central.write(richTextBox1.Text, Central.socks[k]);
            if (Central.read(Central.socks[k]) != Central.answers[0])
            {
                MessageBox.Show(Central.read(Central.socks[k]), "Message from server.");
                Central.socks[k].Close();
                Central.set_right(k);
            }
            else {
                //Central.searching_panel.Controls.RemoveByKey(u.Name);
                this.Close();
            }
        }

        bool shiftDown;
        delegate void no_params();
        private void richTextBox1_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == Keys.Shift || e.KeyCode == Keys.ShiftKey) shiftDown = true;
            if (e.KeyCode == Keys.Enter && !shiftDown) { add(); }
        }
        private void richTextBox1_KeyUp(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == Keys.Shift || e.KeyCode == Keys.ShiftKey) shiftDown = false;
        }
        private void richTextBox1_Leave(object sender, EventArgs e)
        {
            shiftDown = false;
        }
        private void button1_Click(object sender, EventArgs ev)
        {
            add();
        }
    }
}
