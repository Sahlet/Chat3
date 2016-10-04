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
    public partial class Conference : UserControl{

        public Conference_fields fields;

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
        
        public Conference(string id, string name, ContextMenuStrip contextMenuStrip){
            InitializeComponent();
            this.Dock = DockStyle.Top;
            this.fields = Conference_fields.getConference_fields(id, name);
            this.ContextMenuStrip = contextMenuStrip;
            this.Unread = this.fields.Unread;
            this.Name = this.fields.ID;
            this.name.Text = this.fields.Name;
            this.fields.UnreadChanged.Add(new method_empty(this.UnreadChanged));
            this.fields.Last_tickChanged.Add(new method_empty(this.Last_tickChanged));
        }

        public virtual Conference copy() {
            return new Conference(fields.ID, fields.Name, this.ContextMenuStrip);
        }

        protected virtual void Contact_Click(object sender, EventArgs e){
            Central.ShowMessages(this.fields);
        }
        public void UnreadChanged() {
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
                        Conference C = (Conference)this.Parent.Controls[i];
                        if (DateTime.Parse(C.fields.Last_tick) >= this_last_tick)
                        {
                            this.Parent.Controls.SetChildIndex(this, i);
                            break;
                        }
                    }
                    catch (Exception) { }
                }
            }
        }
    }
}
