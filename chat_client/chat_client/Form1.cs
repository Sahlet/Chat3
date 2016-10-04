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
using System.Threading;

namespace chat_client
{
    public delegate void method(object sender, EventArgs e);
    public delegate void method_empty();
    public partial class Form1 : Form
    {   
#region _._._._.Готово
        ////////////////////////////////////////////////////////////////
        
        
        ////////////////////////////////////////////////////////////////
        //для управления расположения панелей на форме
        int x;
        private void panel11_MouseDown(object sender, MouseEventArgs e)
        {
            x = e.X;
        }
        private void panel11_MouseMove(object sender, MouseEventArgs e)
        {
            if (e.Button == MouseButtons.Left && e.X != x)
            {
                if ((panel11.Location.X + e.X - x >= menuStrip1.Size.Width / 4) && ((panel11.Location.X + e.X - x) + panel11.Size.Width <= (menuStrip1.Size.Width * 4) / 10))
                    panel11.Location = new Point(panel11.Location.X + e.X - x, panel11.Location.Y);
                else
                {
                    if (panel11.Location.X + e.X - x < menuStrip1.Size.Width / 4)
                        panel11.Location = new Point(menuStrip1.Size.Width / 4, panel11.Location.Y);
                    else
                        panel11.Location = new Point((menuStrip1.Size.Width * 4) / 10 - panel11.Size.Width, panel11.Location.Y);
                }
            }
        }
        private void panel11_Move(object sender, EventArgs e)
        {
            panel1.Size = new Size(panel11.Location.X, panel1.Size.Height);
            panel2.Size = new Size(menuStrip1.Size.Width - (panel11.Location.X + panel11.Size.Width), panel2.Size.Height);
            panel2.Location = new Point(panel11.Location.X + panel11.Size.Width, menuStrip1.Size.Height);
        }

        ////////////////////////////////////////////////////////////////
        private void toolStripMenuItem5_Click(object sender, EventArgs e){
            this.Close();
            this.Dispose();
            Central.Main();
        }
        ////////////////////////////////////////////////////////////////
        //управление воодом сообщения
        bool shiftDown;
        delegate void no_params();
        private void richTextBox2_KeyDown(object sender, KeyEventArgs e){
            if (e.KeyCode == Keys.Shift || e.KeyCode == Keys.ShiftKey) shiftDown = true;
            if (e.KeyCode == Keys.Enter && !shiftDown) {
                (new System.Threading.Thread(new ParameterizedThreadStart(Central.send_mess))).Start(this.richTextBox2.Text);
                (new System.Threading.Thread(delegate() { this.BeginInvoke(new no_params(richTextBox2.Clear)); })).Start();
            }
        }
        private void richTextBox2_KeyUp(object sender, KeyEventArgs e){
            if (e.KeyCode == Keys.Shift || e.KeyCode == Keys.ShiftKey) shiftDown = false;
        }
        private void richTextBox2_Leave(object sender, EventArgs e){
            shiftDown = false;
        }
        private void button1_Click(object sender, EventArgs ev){
            Central.send_mess(this.richTextBox2.Text);
        }
        ////////////////////////////////////////////////////////////////
        private void toolStripMenuItem6_Click(object sender, EventArgs e){
            Central.refresh_data();
        }
        ////////////////////////////////////////////////////////////////
        private void textBox1_TextChanged(object sender, EventArgs e)
        {
            search();
        }
        private void textBox1_KeyPress(object sender, KeyPressEventArgs e)
        {
            if (e.KeyChar == '\r') search();
        }
        private void button2_Click(object sender, EventArgs e)
        {
            textBox1.Text = "";
        }
        private void button9_Click(object sender, EventArgs e)
        {
            search();
        }
        private void tabControl1_SelectedIndexChanged(object sender, EventArgs e)
        {
            foreach (Control obj in panel5.Controls) { if (obj.Dock != DockStyle.Top) panel5.Controls.Remove(obj); }
            panel5.Controls.Add((Control)tabControl1.SelectedTab.Tag);
            Central.load_more_to(tabControl1.SelectedTab.Tag, null);
        }
        void search()
        {
            //ItemList_with_Button_more list = (ItemList_with_Button_more)tabControl1.SelectedTab.Tag;
            //list.Clear();
            Central.load_more_to(tabControl1.SelectedTab.Tag, null);
        }
        ////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////

        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            Central.Close();
        }

        private void toolStripMenuItem1_Click(object sender, EventArgs e)
        {
            User u = (User)((ContextMenuStrip)(((ToolStripMenuItem)sender).GetCurrentParent())).SourceControl;
            Label label = new Label();
            label.Text = "\n\nStatus:\n" + Central.get_user_status(u.Name);
            Form3 form = new Form3(u, label);
            form.ShowDialog();
        }

        private void toolStripMenuItem11_Click(object sender, EventArgs e)
        {
            User u = (User)((ContextMenuStrip)(((ToolStripMenuItem)sender).GetCurrentParent())).SourceControl;
            Label label = new Label();
            label.Text = "\n\nStatus:\n" + Central.get_user_status(u.Name);
            if (u.Tag != null) label.Text += "\n\nMessage:\n" + (string)u.Tag;
            Form3 form = new Form3(u, label);
            form.ShowDialog();
        }

        private void toolStripMenuItem2_Click(object sender, EventArgs e)
        {
            Conference c = (Conference)((ContextMenuStrip)(((ToolStripMenuItem)sender).GetCurrentParent())).SourceControl;
            Form3 form = new Form3(c, c.fields.partners);
            form.ShowDialog();
        }

        private void перепискаToolStripMenuItem_Click(object sender, EventArgs e)
        {
            try { Central.ShowMessages(User_fields.Users[((User)(((ContextMenuStrip)(((ToolStripMenuItem)sender).GetCurrentParent())).SourceControl)).Name]); }
            catch(Exception) { Central.ShowMessages(User_fields.Users[((Conference)(((ContextMenuStrip)(((ToolStripMenuItem)sender).GetCurrentParent())).SourceControl)).Name]); }
        }

        private void toolStripMenuItem9_Click(object sender, EventArgs e)
        {
            try { Central.ShowMessages(User_fields.Users[((User)(((ContextMenuStrip)(((ToolStripMenuItem)sender).GetCurrentParent())).SourceControl)).Name]); }
            catch (Exception) { Central.ShowMessages(User_fields.Users[((Conference)(((ContextMenuStrip)(((ToolStripMenuItem)sender).GetCurrentParent())).SourceControl)).Name]); }
        }

        private void toolStripMenuItem13_Click(object sender, EventArgs e)
        {
            User u = (User)((ContextMenuStrip)(((ToolStripMenuItem)sender).GetCurrentParent())).SourceControl;
            if (!Central.requests.Controls.Contains(u))
            {
                AdderFriend form = new AdderFriend(u);
                form.Show();
            }
            else accept(u);
        }

        void accept(User u)
        {
            Waiter w = new Waiter(ref Central.counter[(int)Central.key.accepter_request, 0], ref Central.counter[(int)Central.key.accepter_request, 1], new method_obj1(this.accept_), u);
        }
        object accept_(object u_)
        {
            User u = (User)u_;
            int k = (int)Central.key.accepter_request;
            if (Central.socks[k] == null) Central.set_right(k);
            if (Central.socks[k] == null) Central.Fatal_Error();
            Central.write(u.Name, Central.socks[k]);
            if (Central.read(Central.socks[k]) != Central.answers[0])
            {
                MessageBox.Show(Central.read(Central.socks[k]), "Message from server.");
                Central.socks[k].Close();
                Central.set_right(k);
            }
            Central.requests.Controls.RemoveByKey(u.Name);
            Central.searching_panel.Controls.RemoveByKey(u.Name);
            return null;
        }
#endregion

        public Form1(){

            InitializeComponent();

            this.Text += ": " + Central.log;

            tabPage1.Tag = Central.friends;
            tabPage2.Tag = Central.conferences;
            tabPage3.Tag = Central.searching_panel;
            tabPage4.Tag = Central.requests;

            this.panel5.Controls.Add(Central.friends);
        }

    }
}
