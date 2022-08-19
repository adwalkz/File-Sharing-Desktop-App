#include<gtk/gtk.h>
#include<string.h>
#include <libssh/libssh.h>

//_________________________________________________________________________________________________

GtkWidget *FWin, *FWA;
GtkWidget *Flab, *Fb1, *Fb2;
GtkWidget *window, *workArea;
GtkWidget *popwin, *wA, *cross;
GtkWidget *b1, *b2;
GtkWidget *lab, *text, *but;
GtkWidget *FileName, *Usr, *IPAdd, *proBar;

//_________________________________________________________________________________________________

const char *fpath = "Choose File";
const char *password;
int sockfd, connfd, W=-1;
gdouble progress = 0.0;

//_________________________________________________________________________________________________

void clear()
{
	gtk_entry_set_text(GTK_ENTRY(FileName), "");
	gtk_entry_set_text(GTK_ENTRY(Usr), "");
	gtk_entry_set_text(GTK_ENTRY(IPAdd), "");
	
	if(W == 0)
		gtk_widget_grab_focus(b1);
	else
		gtk_widget_grab_focus(FileName);
}

/*
This function
create Fixed Area
*/
void newWA()
{
	workArea = gtk_fixed_new();
	gtk_container_add(GTK_CONTAINER(window), workArea);
}

/*
This function
show First Page
*/
void showFP()
{
	gtk_widget_show(FWin);
	
	if(W != -1)
		gtk_widget_hide(window);
}

/*
This function
hide First Page
*/
void hideFP()
{
	gtk_widget_hide(FWin);
}

/*
This function
create new window
*/
void newWin()
{
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(window), 600, 100);
	gtk_window_set_title(GTK_WINDOW(window), "FILE SHARE");
	gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
	gtk_window_set_modal(GTK_WINDOW(window), FALSE);
	
	g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);

	newWA();
}

/*
This function
create a TOPLEVEL (used as POPUP) window
*/
void newPop()
{
	popwin = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(popwin), 300, 100);
	gtk_window_set_position(GTK_WINDOW(popwin), GTK_WIN_POS_CENTER);
	gtk_window_set_decorated(GTK_WINDOW(popwin), FALSE);
	
	wA = gtk_fixed_new();
	gtk_container_add(GTK_CONTAINER(popwin), wA);
	
	gtk_widget_show_all(popwin);
}

/*
This function
destroy a TOPLEVEL (used as POPUP) window
*/
void desPop()
{
	gtk_widget_destroy(popwin);
	clear();
}

/*
This function
choose file
*/
void ChFile()
{
	GtkWidget *FileDialog;
	GtkFileChooser *ch;
	
	FileDialog = gtk_file_chooser_dialog_new ("Select file to send", GTK_WINDOW(window), 				GTK_FILE_CHOOSER_ACTION_OPEN, ("_Cancel"), GTK_RESPONSE_CANCEL, ("_Select"), 				GTK_RESPONSE_ACCEPT, NULL);

	gint res = gtk_dialog_run(GTK_DIALOG(FileDialog));
	
	if(res == GTK_RESPONSE_ACCEPT)
	{
		ch = GTK_FILE_CHOOSER(FileDialog);
		fpath = gtk_file_chooser_get_filename(ch);
	}
	
	gtk_widget_destroy(FileDialog);
	gtk_entry_set_text(GTK_ENTRY(FileName), fpath);
}

gboolean update()
{
	progress += 0.3;
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(proBar), progress);
	
	if(progress > 1.0)
	{	
		desPop();
		newPop();
		
		lab = gtk_label_new(NULL);
		gtk_label_set_text(GTK_LABEL(lab), "SUCC: File Writing Success");
		gtk_fixed_put(GTK_FIXED(wA), lab, 10, 50);
		
		cross = gtk_button_new_with_label("X");
		gtk_fixed_put(GTK_FIXED(wA), cross, 270, 3);
		g_signal_connect(G_OBJECT(cross), "clicked", G_CALLBACK(desPop), NULL);
		
		gtk_widget_show_all(popwin);
		
		return FALSE;
	}

	return TRUE;
}

void status()
{
	gtk_widget_destroy(popwin);
	
	newPop();
	
	proBar = gtk_progress_bar_new();
	gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(proBar), TRUE);
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(proBar), "Sending Status");
	gtk_fixed_put(GTK_FIXED(wA), proBar, 80, 20);
	
	gtk_widget_show_all(popwin);
}

void succ()
{
	g_timeout_add(500, update, NULL);
}

/*
Main Logic
to PUSH a file
*/
void Write()
{
	int port = 22;
	int rc;
	
	fpath = gtk_entry_get_text(GTK_ENTRY(FileName));
	password = gtk_entry_get_text(GTK_ENTRY(text));

	FILE *fp = fopen(fpath, "r");
	fseek(fp,0,SEEK_END);
	int size = ftell(fp);
	fseek(fp,0,SEEK_SET);
	char *t = malloc(size);
	fread(t, 1, size, fp);
	fclose(fp);

	ssh_session TSession = ssh_new();
	ssh_init();
	if(TSession == NULL)
	{
		printf("Error: initialising sessoin FAILED\n");
		status();
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(proBar), 0.1);
		gtk_label_set_text(GTK_LABEL(lab), "Error: Initialising Sessoin Failed");
	}
	else
	{
		printf(">> Initialisation Success\n");
		
		ssh_options_set(TSession, SSH_OPTIONS_HOST, gtk_entry_get_text(GTK_ENTRY(IPAdd)));
		ssh_options_set(TSession, SSH_OPTIONS_PORT, &port);
		ssh_options_set(TSession, SSH_OPTIONS_USER, gtk_entry_get_text(GTK_ENTRY(Usr)));

		rc = ssh_connect(TSession);
		if(rc != SSH_OK)
		{
			printf("Error: SSH CONNECTION Failed\n");
			status();
			gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(proBar), 0.2);
			gtk_label_set_text(GTK_LABEL(lab), "Error: SSH CONNECTION Failed");
		}
		else
		{
			printf(">> SSH CONNECTION Success\n");
			rc = ssh_userauth_password(TSession, NULL, password);
			if(rc == SSH_AUTH_ERROR)
			{
				printf("Error: Authentication Failed\n");
				status();
				gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(proBar), 0.3);
				gtk_label_set_text(GTK_LABEL(lab), "Error: SSH CONNECTION Failed");
			}
			else
			{
				printf(">> Authentication Success\n");
				ssh_scp scp = ssh_scp_new(TSession, SSH_SCP_WRITE, ".");
				if(scp == NULL)
				{
					printf("Error: SCP Session Allocation Failed\n");
					status();
					gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(proBar), 0.4);
					gtk_label_set_text(GTK_LABEL(lab), "Error: SSH CONNECTION Failed");
				}
				else
				{
					printf(">> SCP Session Allocation Success\n");
					
					rc = ssh_scp_init(scp);
					if(rc != SSH_OK)
					{
						printf("Error: SCP Session Initialisation Failed\n");
						status();
						gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(proBar), 0.5);
						gtk_label_set_text(GTK_LABEL(lab), "Error: SCP Session Initialisation Failed");
					}
					else
					{
						printf(">> SCP Session Initialisation Success\n");
							
						rc = ssh_scp_push_file(scp, fpath, size, 777);
						if(rc != SSH_OK)
						{
							printf("Error: File Open Failed\n");
							status();
							gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(proBar), 0.6);
							gtk_label_set_text(GTK_LABEL(lab), "Error: SSH CONNECTION Failed");
						}
						else
						{
							printf(">> File Open Success\n");
							rc = ssh_scp_write(scp, t, size);
							if(rc != SSH_OK)
							{
								printf("Error: Writing into File Failed\n");
								status();
								gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(proBar), 0.9);
								gtk_label_set_text(GTK_LABEL(lab), "Error: SSH CONNECTION Failed");
							}
							else
							{
								printf(">> File writing Success\n");
								status();
								succ();
								gtk_widget_show_all(popwin);
							}
						}
						free(t);
						ssh_scp_close(scp);
						ssh_scp_free(scp);
					}
				}
			}
		}
	}
	ssh_free(TSession);
}

/*
Main Logic
to PULL a file
*/
void Read()
{
	int port = 22;
	int rc, size;
	char *buffer;
	
	fpath = gtk_entry_get_text(GTK_ENTRY(FileName));
	password = gtk_entry_get_text(GTK_ENTRY(text));
	
	ssh_session TSession = ssh_new();
	if(TSession == NULL)
	{
		printf("Error: initialising sessoin FAILED\n");
	}
	else
	{
		printf(">> Initialisation Success\n");
		
		ssh_options_set(TSession, SSH_OPTIONS_HOST, gtk_entry_get_text(GTK_ENTRY(IPAdd)));
		ssh_options_set(TSession, SSH_OPTIONS_PORT, &port);
		ssh_options_set(TSession, SSH_OPTIONS_USER, gtk_entry_get_text(GTK_ENTRY(Usr)));

		rc = ssh_connect(TSession);
		if(rc != SSH_OK)
		{
			printf("Error: SSH CONNECTION Failed\n");
		}
		else
		{
			printf(">> SSH CONNECTION Success\n");
			rc = ssh_userauth_password(TSession, NULL, password);
			if(rc == SSH_AUTH_ERROR)
			{
				printf("Error: Authentication Failed\n");
			}
			else
			{
				printf(">> Authentication Success\n");

				ssh_scp scp = ssh_scp_new(TSession, SSH_SCP_READ, fpath);
				if(scp == NULL)
				{
					printf("Error: SCP Session Allocation Failed\n");
				}
				else
				{
					printf(">> SCP Session Allocation Success\n");
					
					rc = ssh_scp_init(scp);
					if(rc != SSH_OK)
					{
						printf("Error: SCP Session Initialisation Failed\n");
					}
					else
					{
						printf(">> SCP Session Initialisation Success\n");
							
						rc = ssh_scp_pull_request(scp);
						if(rc != SSH_SCP_REQUEST_NEWFILE)
						{
							printf("Error: File Open Failed\n");
						}
						else
						{
							printf(">> File Open Success\n");
							ssh_scp_accept_request(scp);
							size = ssh_scp_request_get_size(scp);
							buffer = malloc(size);
							if (buffer == NULL)
							{
								printf("Memory allocation error\n");
							}
							rc = ssh_scp_read(scp, buffer, size);
							if (rc == SSH_ERROR)
							{
								printf("Error: Receiving File Data Failed\n");
							}
							else
							{
								printf("buffer: %s\n", buffer);
							}
						}
						ssh_scp_close(scp);
						ssh_scp_free(scp);
					}
				}
			}
		}
	}
	ssh_free(TSession);
}

void popup()
{
	newPop();

	lab = gtk_label_new("Enter Password: ");
	gtk_fixed_put(GTK_FIXED(wA), lab, 5, 18);
	
	text = gtk_entry_new();
	gtk_entry_set_placeholder_text(GTK_ENTRY(text), "Password");
	gtk_entry_set_visibility(GTK_ENTRY(text), FALSE);
	gtk_entry_set_invisible_char(GTK_ENTRY(text), 42);
	gtk_fixed_put(GTK_FIXED(wA), text, 120, 15);

	gtk_widget_grab_focus(text);

	GtkWidget *but1 = gtk_button_new_with_label("DONE");
	gtk_fixed_put(GTK_FIXED(wA), but1, 120, 50);
	if(W==0)
		g_signal_connect(G_OBJECT(but1), "clicked", G_CALLBACK(Write), NULL);
	else
		g_signal_connect(G_OBJECT(but1), "clicked", G_CALLBACK(Read), NULL);
	
	GtkWidget *but2 = gtk_button_new_with_label("CANCEL");
	gtk_fixed_put(GTK_FIXED(wA), but2, 200, 50);
	g_signal_connect(G_OBJECT(but2), "clicked", G_CALLBACK(desPop), NULL);

	gtk_widget_show_all(popwin);
}

/*
This function
Open menu for Pushing File
to server
*/
void PushFile()
{
	hideFP();
	newWin();
	
	W = 0;
	
	GtkWidget *Back = gtk_button_new_with_label("BACK");
	gtk_fixed_put(GTK_FIXED(workArea), Back, 10, 10);
	g_signal_connect(G_OBJECT(Back), "clicked", G_CALLBACK(showFP), NULL);
	
	FileName = gtk_entry_new();
	gtk_entry_set_placeholder_text(GTK_ENTRY(FileName), "Choose File");
	gtk_widget_set_size_request(FileName, 450, 5);
	gtk_widget_set_sensitive(FileName, FALSE);
	gtk_fixed_put(GTK_FIXED(workArea), FileName, 65, 10);
	
	
	b1 = gtk_button_new_with_label("BROWSE");
	gtk_fixed_put(GTK_FIXED(workArea), b1, 520, 10);
	g_signal_connect(G_OBJECT(b1), "clicked", G_CALLBACK(ChFile), NULL);

	Usr = gtk_entry_new();
	gtk_entry_set_placeholder_text(GTK_ENTRY(Usr), "Enter UserName");
	gtk_widget_set_size_request(Usr, 250, 5);
	gtk_fixed_put(GTK_FIXED(workArea), Usr, 10, 50);

	IPAdd = gtk_entry_new();
	gtk_entry_set_placeholder_text(GTK_ENTRY(IPAdd), "Enter IP Address");
	gtk_widget_set_size_request(IPAdd, 250, 5);
	gtk_fixed_put(GTK_FIXED(workArea), IPAdd, 265, 50);

	b2 = gtk_button_new_with_label("SEND");
	gtk_fixed_put(GTK_FIXED(workArea), b2, 520, 50);
	g_signal_connect_swapped(G_OBJECT(b2), "clicked", G_CALLBACK(popup), NULL);
	
	gtk_widget_show_all(window);
	
	gtk_entry_set_text(GTK_ENTRY(Usr), "root");
	gtk_entry_set_text(GTK_ENTRY(IPAdd), "192.168.6.193");
}

/*
This function
Open menu for Pulling File
from server
*/
void PullFile()
{
	hideFP();
	newWin();
	
	W = 1;
	
	GtkWidget *Back = gtk_button_new_with_label("BACK");
	gtk_fixed_put(GTK_FIXED(workArea), Back, 10, 10);
	g_signal_connect(G_OBJECT(Back), "clicked", G_CALLBACK(showFP), NULL);
	
	FileName = gtk_entry_new();
	gtk_entry_set_placeholder_text(GTK_ENTRY(FileName), "Choose File");
	gtk_widget_set_size_request(FileName, 525, 5);
	gtk_fixed_put(GTK_FIXED(workArea), FileName, 65, 10);
	
	gtk_widget_grab_focus(FileName);
	
	Usr = gtk_entry_new();
	gtk_entry_set_placeholder_text(GTK_ENTRY(Usr), "Enter UserName");
	gtk_widget_set_size_request(Usr, 250, 5);
	gtk_fixed_put(GTK_FIXED(workArea), Usr, 5, 50);

	IPAdd = gtk_entry_new();
	gtk_entry_set_placeholder_text(GTK_ENTRY(IPAdd), "Enter IP Address");
	gtk_widget_set_size_request(IPAdd, 250, 5);
	gtk_fixed_put(GTK_FIXED(workArea), IPAdd, 255, 50);

	b2 = gtk_button_new_with_label("CONNECT");
	gtk_fixed_put(GTK_FIXED(workArea), b2, 510, 50);
	g_signal_connect(G_OBJECT(b2), "clicked", G_CALLBACK(popup), NULL);
	
	gtk_widget_show_all(window);
	
	gtk_entry_set_text(GTK_ENTRY(Usr), "root");
	gtk_entry_set_text(GTK_ENTRY(IPAdd), "192.168.6.193");
}

/*
This function
create First Page
*/
void FirstPage()
{
	FWin = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(FWin), 600, 100);
	gtk_window_set_title(GTK_WINDOW(FWin), "FILE SHARE");
	gtk_window_set_resizable(GTK_WINDOW(FWin), FALSE);
	gtk_window_set_modal(GTK_WINDOW(FWin), FALSE);
	
	g_signal_connect(G_OBJECT(FWin), "destroy", G_CALLBACK(gtk_main_quit), NULL);
	
	FWA = gtk_fixed_new();
	gtk_container_add(GTK_CONTAINER(FWin), FWA);
	
	Flab = gtk_label_new("Choose Action:");
	gtk_fixed_put(GTK_FIXED(FWA), Flab, 50, 20);

	Fb1 = gtk_button_new_with_label("Push File");
	gtk_fixed_put(GTK_FIXED(FWA), Fb1, 250, 50);
	g_signal_connect(G_OBJECT(Fb1), "clicked", G_CALLBACK(PushFile), NULL);
	
	Fb2 = gtk_button_new_with_label("Pull File");
	gtk_fixed_put(GTK_FIXED(FWA), Fb2, 400, 50);
	g_signal_connect(G_OBJECT(Fb2), "clicked", G_CALLBACK(PullFile), NULL);
	
	gtk_widget_show_all(FWin);
}

//_________________________________________________________________________________________________

int main()
{
	gtk_init(NULL, NULL);
	FirstPage();
	gtk_main();
	return 0;
}

