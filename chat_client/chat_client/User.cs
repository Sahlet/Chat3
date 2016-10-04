using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace chat_client
{
    public partial class User : UserControl
    {
        public static User getUser(string login){
            return User_fields.getUser_fields(login).getUser();
        }
        private User_fields fields;

        public bool Unread{
            get {
                if (panel1.BackColor == SystemColors.ControlLight) return false;
                else return true;
            }
            set {
                if (value) panel1.BackColor = SystemColors.ControlDark;
                else panel1.BackColor = SystemColors.ControlLight;
            }
        }
        public bool Online
        {
            get
            {
                return this.online.Visible;
            }
            set
            {
                this.online.Visible = value;
            }
        }
        
        public User(string id, string name, ContextMenuStrip contextMenuStrip){
            InitializeComponent();
            this.Dock = DockStyle.Top;
            this.fields = User_fields.getUser_fields(id, name);
            this.ContextMenuStrip = contextMenuStrip;
            this.Unread = this.fields.Unread;
            this.Online = this.fields.Online;
            this.Name = this.fields.Name;
            this.name.Text = this.fields.Name;
            this.fields.UnreadChanged.Add(new method_empty(this.UnreadChanged));
            this.fields.Last_tickChanged.Add(new method_empty(this.Last_tickChanged));
            this.fields.OnlineChanged.Add(new method_empty(this.OnlineChanged));
        }
        public User(User_fields fields, ContextMenuStrip contextMenuStrip)
        {
            InitializeComponent();
            this.Dock = DockStyle.Top;
            this.fields = fields;
            this.ContextMenuStrip = contextMenuStrip;
            this.Unread = this.fields.Unread;
            this.Online = this.fields.Online;
            this.Name = this.fields.Name;
            this.name.Text = this.fields.Name;
            this.fields.UnreadChanged.Add(new method_empty(this.UnreadChanged));
            this.fields.Last_tickChanged.Add(new method_empty(this.Last_tickChanged));
            this.fields.OnlineChanged.Add(new method_empty(this.OnlineChanged));
        }

        public virtual User copy() {
            return new User(fields.ID, fields.Name, this.ContextMenuStrip);
        }

        protected virtual void Contact_Click(object sender, EventArgs e){
            if (this.fields.ID != null) Central.ShowMessages(this.fields);
        }
        public void UnreadChanged()
        {
            this.Unread = fields.Unread;
        }
        public void Last_tickChanged(){
            if (this.Parent != null)
            {

                int n = this.Parent.Controls.Count;
                DateTime this_last_tick = DateTime.Parse(this.fields.Last_tick);
                for (int i = 0; i < n; i++)
                {
                    try
                    {
                        User U = (User)this.Parent.Controls[i];
                        if (DateTime.Parse(U.fields.Last_tick) >= this_last_tick)
                        {
                            this.Parent.Controls.SetChildIndex(this, i);
                            break;
                        }
                    }
                    catch (Exception) { }
                }
            }
        }
        public void OnlineChanged() {
            this.Online = fields.Online;
        }
    }
}
