/*
 * @(#)UpdatePanel.java	1.17 03/04/10
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.panel;

/**
 * The Update Panel shows the information about getting
 * Java Update.
 *
 * @author	Stanley Man-Kit Ho
 *
 */

import java.awt.GridBagLayout;
import java.awt.GridBagConstraints;
import java.awt.Insets;
import java.awt.Dimension;
import java.awt.FlowLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.Image;
import javax.swing.*;
import javax.swing.border.*;
import javax.swing.SpinnerNumberModel.*;
import javax.swing.event.*;
import java.text.MessageFormat;
import java.text.DateFormatSymbols;
import java.util.Calendar;
import java.util.Date;
import java.util.GregorianCalendar;
import java.util.Locale;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import sun.plugin.util.DialogFactory;
import sun.plugin.services.WinRegistry;
import java.io.File;

public class UpdatePanel extends ActivatorSubPanel
			 implements ActionListener{

    private class AdvancedDialog implements ActionListener {

	public AdvancedDialog(UpdatePanel updPanel, ConfigurationInfo model, MessageHandler mh) {

            JPanel advancedPanel = new JPanel();
            advancedPanel.setLayout(new GridBagLayout());
            c = new GridBagConstraints();

            c.anchor = GridBagConstraints.WEST;
            c.fill = GridBagConstraints.NONE;
            c.insets = new Insets(5, 0, 0, 0);
            c.weighty = 1;
            c.weightx = 0;
            c.gridwidth = GridBagConstraints.REMAINDER;

            advancedPanel.add(new JLabel(mh.getMessage("advanced_title1.text")), c);
            JPanel FreqPanel = new JPanel();
            FreqPanel.setLayout(new BoxLayout(FreqPanel, BoxLayout.Y_AXIS));
            Border border = BorderFactory.createEtchedBorder();
            FreqPanel.setBorder(BorderFactory.createCompoundBorder(BorderFactory.createEmptyBorder(4, 4, 4, 4),
                                BorderFactory.createTitledBorder(border, mh.getMessage("advanced_title2.text"))));

            RButton1 = new JRadioButton(mh.getMessage("check_daily.text"));
            RButton2 = new JRadioButton(mh.getMessage("check_weekly.text"));
            RButton3 = new JRadioButton(mh.getMessage("check_monthly.text"));
            FreqGroup = new ButtonGroup();
            FreqGroup.add(RButton1);
            FreqGroup.add(RButton2);
            FreqGroup.add(RButton3);
            
            RButton1.addActionListener(this);
            RButton2.addActionListener(this);
            RButton3.addActionListener(this);
            FreqPanel.add(RButton1);
            FreqPanel.add(RButton2);
            FreqPanel.add(RButton3);

            c.weighty = 3;
            c.weightx = 0;
            c.gridwidth = 2;
            c.insets = new Insets(0, 0, 0, 0);
            advancedPanel.add(FreqPanel, c);

            whenPanel = new JPanel();
            whenPanel.setLayout(new GridBagLayout());
            whenPanel.setBorder(BorderFactory.createCompoundBorder(BorderFactory.createEmptyBorder(4, 4, 4, 4),
                                BorderFactory.createTitledBorder(border, mh.getMessage("advanced_title3.text"))));

            atComboBoxText = updPanel.getAtComboBoxText();
            weekDays = updPanel.getWeekDays();

            dummyLabel = new JLabel("    ");
            everyLabelText = mh.getMessage("check_day.text");
            atLabelText = mh.getMessage("check_time.text");
            dayLabelText = mh.getMessage("check_date.text");
            whenLabel1 = new JLabel();
            whenLabel2 = new JLabel();

            every = new JComboBox(); 
            every.setMaximumRowCount(7);
            every.addItem(weekDays[Calendar.SUNDAY]);
            every.addItem(weekDays[Calendar.MONDAY]);
            every.addItem(weekDays[Calendar.TUESDAY]);
            every.addItem(weekDays[Calendar.WEDNESDAY]);
            every.addItem(weekDays[Calendar.THURSDAY]);
            every.addItem(weekDays[Calendar.FRIDAY]);
            every.addItem(weekDays[Calendar.SATURDAY]);
            every.addActionListener(this);

            at = new JComboBox(); 
            at.setMaximumRowCount(atComboBoxText.length);
            for (int i=0; i < atComboBoxText.length; i++) {
                at.addItem(atComboBoxText[i]);
            }
            at.addActionListener(this);
    
            daymodel = new SpinnerNumberModel(1, 1, 31, 1); 
            day = new JSpinner(daymodel);
            daymodel.addChangeListener(changeListener);

            c.anchor = GridBagConstraints.WEST;

            whenPanel1 = new JPanel();
            whenPanel2 = new JPanel();
            c.insets = new Insets(0, 10, 0, 5);
            c.gridwidth = 2;
            c.gridheight = 2;
            c.weighty = 0;
            c.weightx = 0;
            whenPanel.add(whenLabel1, c);
            c.gridwidth = GridBagConstraints.REMAINDER;
            c.weightx = 1;
            c.fill = GridBagConstraints.HORIZONTAL;
            whenPanel.add(whenPanel1, c);
            c.gridwidth = 2;
            c.fill = GridBagConstraints.NONE;
            c.weightx = 0;
            c.gridheight = GridBagConstraints.REMAINDER;
            whenPanel.add(whenLabel2, c);
            c.weightx = 1;
            c.fill = GridBagConstraints.HORIZONTAL;
            c.gridwidth = GridBagConstraints.REMAINDER;
            whenPanel.add(whenPanel2, c);
    
            c.weightx = 2;
            c.weighty = 3;
            c.fill = GridBagConstraints.BOTH;
            c.gridheight = GridBagConstraints.RELATIVE;
            c.insets = new Insets(0, 5, 0, 0);
            c.gridwidth = GridBagConstraints.REMAINDER;
            advancedPanel.add(whenPanel, c);

            if (model.getUpdateFrequency() == 0) 
            {
                RButton1.setSelected(true);
            }
            else if (model.getUpdateFrequency() == 2) 
            {
                RButton3.setSelected(true);
                day.setValue(new Integer(model.getUpdateDay()));
            }
            else {
                RButton2.setSelected(true);
                every.setSelectedIndex(model.getUpdateDay()-1);
            }
            at.setSelectedIndex(model.getUpdateSchedule());
            
            descLabel = new JLabel("  ");
            descLabel.setBorder(BorderFactory.createCompoundBorder(BorderFactory.createEmptyBorder(4, 4, 4, 4),
                                BorderFactory.createTitledBorder(border, "")));
            c.fill = GridBagConstraints.HORIZONTAL;
            c.weightx = 1;
            c.weighty = 1;
            c.insets = new Insets(0, 0, 0, 0);
            c.gridwidth = GridBagConstraints.REMAINDER;
            advancedPanel.add(descLabel, c);
             
            setPanelOptions();

            JOptionPane pane = new JOptionPane(advancedPanel, 
                                               JOptionPane.PLAIN_MESSAGE,
                                               JOptionPane.OK_CANCEL_OPTION);
            JDialog dialog = pane.createDialog(null, 
                                         mh.getMessage("advanced_title.text"));
            dialog.setModal(true);
            dialog.setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);
            dialog.show();
            Integer value = (Integer) pane.getValue();
            if (value != null && value.intValue()==JOptionPane.OK_OPTION) {
                    model.setUpdateSchedule(at.getSelectedIndex());
                    if (RButton1.isSelected()) 
                    {
                        model.setUpdateFrequency(0);
                        model.setUpdateDay(0);
                    }
                    else if (RButton3.isSelected())
                    {
                        model.setUpdateFrequency(2);
                        model.setUpdateDay(((Integer)day.getValue()).intValue());
                    }
                    else {
                        model.setUpdateFrequency(1);
                        model.setUpdateDay(every.getSelectedIndex()+1);
                    }
             }
             updPanel.setText();
        }

        void reset() {
        }

        void setPanelOptions() {
            String str = null;
            String str1 = null;
            MessageFormat msgfmt;
 
            if (RButton1.isSelected()) {
                day.setVisible(false);
                every.setVisible(false);
                whenLabel1.setText("  ");
                whenLabel2.setText(atLabelText);
                whenPanel1.add(dummyLabel);
                whenPanel2.add(at);
                at.setVisible(true);
                dummyLabel.setVisible(true);
                str = mh.getMessage("advanced_desc1.text");
                msgfmt = new MessageFormat(str);
                str = msgfmt.format(new String[] { (String)at.getItemAt(at.getSelectedIndex()) });
            } else if (RButton2.isSelected()) {
                day.setVisible(false);
                dummyLabel.setVisible(false);
                whenLabel1.setText(everyLabelText);
                whenLabel2.setText(atLabelText);
                whenPanel1.add(every, c);
                whenPanel2.add(at);
                every.setVisible(true);
                at.setVisible(true);
                str1 = (String) every.getItemAt(every.getSelectedIndex());
                str = mh.getMessage("advanced_desc2.text");
            } else if (RButton3.isSelected()) {
                dummyLabel.setVisible(false);
                every.setVisible(false);
                whenLabel1.setText(dayLabelText);
                whenLabel2.setText(atLabelText);
                whenPanel1.add(day);
                whenPanel2.add(at);
                day.setVisible(true);
                at.setVisible(true);
                str1 = String.valueOf(((Integer)day.getValue()).intValue());
                str = mh.getMessage("advanced_desc3.text");
            }
            if (str1 != null) {
                msgfmt = new MessageFormat(str);
                str = msgfmt.format(new String[]{str1, (String)at.getItemAt(at.getSelectedIndex()) });
            }
            descLabel.setText(str);
        }

        public void actionPerformed(ActionEvent e) {
            try {
                if ((e.getSource()==RButton1) ||
                    (e.getSource()==RButton2) ||
                    (e.getSource()==RButton3) ||
                    (e.getSource()==every) ||
                    (e.getSource()==at)) {

                    setPanelOptions();
                }
            } catch(Exception exc) { }
        }

        ChangeListener changeListener = new ChangeListener() {
            public void stateChanged(ChangeEvent e) {
                try {
                    if (e.getSource()==daymodel) setPanelOptions();
                } catch(Exception exc) { }
            }
        };

        private JRadioButton RButton1, RButton2, RButton3;
        private ButtonGroup FreqGroup;
        private String[] weekDays ;
        private String[] atComboBoxText;
        private JComboBox at, every;
        JSpinner day;
        SpinnerNumberModel daymodel;
        private String atLabelText, everyLabelText, dayLabelText;
        private JLabel whenLabel1, whenLabel2, descLabel, dummyLabel;
        JPanel whenPanel1, whenPanel2, whenPanel;
        GridBagConstraints c;
    }


    /**
     * Construct the panel, add widgets
     */
    UpdatePanel(ConfigurationInfo model) {
	super(model);

	setLayout(new BoxLayout(this, BoxLayout.X_AXIS));
	setBorder(BorderFactory.createEmptyBorder(4,4,4,4));

	JPanel imgPanel = new JPanel();
        JLabel imgLabel = new JLabel();
	ImageIcon img = new ImageIcon(ClassLoader.getSystemResource(update_image));
        imgLabel.setIcon((Icon)img);
        imgPanel.add(imgLabel);
        add(imgPanel);

        DateFormatSymbols datefmt = new DateFormatSymbols();
	DateFormat formatter = DateFormat.getTimeInstance(DateFormat.SHORT);
	SimpleDateFormat sdf = new SimpleDateFormat("H:mm");
	Date temp;

        weekDays = datefmt.getWeekdays();
        atComboBoxText = new String[24];
        //start with 12AM
        for (int i=0; i < 24; i++) {
	    try {
                temp = sdf.parse(i+":00"); 
                atComboBoxText[i] = formatter.format(temp);
            } catch (Exception e)  { atComboBoxText[i] = i+":00";}
        }

	JPanel rightPanel = new JPanel();
	rightPanel.setLayout(new GridBagLayout());
	GridBagConstraints c = new GridBagConstraints();

	c.anchor = GridBagConstraints.WEST;
	c.fill = GridBagConstraints.NONE;
	c.insets = new Insets(5, 0, 0, 0);
	c.weighty = 0;
	c.weightx = 0;
	c.gridwidth = GridBagConstraints.REMAINDER;
	
	JLabel label1 = new JLabel(mh.getMessage("desc.text"));
	rightPanel.add(label1, c);

	c.weighty = 2;
	c.gridwidth = 3;
	c.insets = new Insets(10, 0, 0, 5);
        rightPanel.add(new JLabel(mh.getMessage("notify.text")), c);
        notify = new JComboBox(); 
        notify.setMaximumRowCount(3);
        notify.addItem(ntfyInstallText);
        notify.addItem(ntfyDownldText);
        notify.addActionListener(this);
        c.insets = new Insets(10, 0, 0, 15);
	c.gridwidth = GridBagConstraints.RELATIVE;
	c.anchor = GridBagConstraints.WEST;
        rightPanel.add(notify, c);
	c.gridwidth = GridBagConstraints.REMAINDER;
        rightPanel.add(new JLabel(""), c); //dummy label for correct alignment

	c.insets = new Insets(10, 0, 0, 0);
	c.gridwidth = GridBagConstraints.RELATIVE;
        updateCheckBox = new JCheckBox(mh.getMessage("autoupdate.text"));
        updateCheckBox.addActionListener(this);
        rightPanel.add(updateCheckBox, c);

	c.anchor = GridBagConstraints.EAST;
	c.gridwidth = GridBagConstraints.REMAINDER;
	c.insets = new Insets(10, 15, 0, 15);
	advancedButton = new JButton(mh.getMessage("advanced_button.text"));
        advancedButton.addActionListener(this);
	rightPanel.add(advancedButton, c);

	c.gridwidth = 0;
	c.weighty = 3;
	c.insets = new Insets(0, 0, 0, 0);
	c.anchor = GridBagConstraints.WEST;
        textDescLabel = new JLabel(" ");
        rightPanel.add(textDescLabel, c);  //for description string

	c.weighty = 2;
	c.gridwidth = GridBagConstraints.RELATIVE;
        lastRunLabel = new JLabel(" ");
        rightPanel.add(lastRunLabel, c); // for last run string
        
	c.anchor = GridBagConstraints.EAST;
	c.gridwidth = GridBagConstraints.REMAINDER;
	c.insets = new Insets(0, 15, 0, 15);
	updateButton = new JButton(mh.getMessage("update_button.text"));
        updateButton.addActionListener(this);
	rightPanel.add(updateButton, c);

	add(rightPanel);
	reset();
    }
    

    /* Reset all the settings from the model */
    public void reset() {
	updateCheckBox.setSelected(model.getUpdateEnabled());
        notify.setSelectedIndex(model.getUpdateNotify());
        setText();
        String t = model.getUpdateLastRun();
        Calendar c = new GregorianCalendar();
        try {
            c.setTime(df.parse(t));
        } catch (Exception e)  { t = null;System.out.println("got exception:"+e);}
        if (t == null) t = new String("    ");
        else {
            MessageFormat lastRunText;
	    DateFormat timefmt = DateFormat.getTimeInstance(DateFormat.SHORT);
	    DateFormat datefmt = DateFormat.getDateInstance(DateFormat.SHORT);

            lastRunText = new MessageFormat(mh.getMessage("lastrun.text"));

	    String str1 = timefmt.format(c.getTime()); 
	    String str2= datefmt.format(c.getTime()); 
            t = lastRunText.format(new String[]{str1,str2});
        }
	lastRunLabel.setText(t);
    }
    
    // Set textDesc to be displaed at the bottom.
    private void setText() {
        if (!updateCheckBox.isSelected()) {
	    notify.setEnabled(false);
	    advancedButton.setEnabled(false);
	    textDesc = mh.getMessage("desc_autooff.text");
	    textDescLabel.setText(textDesc);
	} else {
	    notify.setEnabled(true);
	    advancedButton.setEnabled(true);
	    int index = model.getUpdateFrequency();
            
            MessageFormat msgfmt;
            String time_str = atComboBoxText[model.getUpdateSchedule()];
            String str1 = null;
	    if (index == 0) {
	        textDesc = mh.getMessage("desc_check_daily.text");
                msgfmt = new MessageFormat(textDesc);
                textDesc = msgfmt.format(new String[] { time_str });                
            }
	    else if (index == 2) {
	        textDesc = mh.getMessage("desc_check_monthly.text");
                str1 = String.valueOf(model.getUpdateDay());
            }
	    else {
	        textDesc = mh.getMessage("desc_check_weekly.text") ;
                str1 = String.valueOf(weekDays[model.getUpdateDay()]);
            }
            if (str1 != null) {
                msgfmt = new MessageFormat(textDesc);
                textDesc = msgfmt.format(new String[]{ str1, time_str });
            }
	    textDesc = textDesc + checkText + notifyText;
	    index = notify.getSelectedIndex();
            if (index == 1) 
	        textDesc = textDesc + mh.getMessage("desc_notify_download.text");
	    else
	        textDesc = textDesc + mh.getMessage("desc_notify_install.text");
	    textDescLabel.setText(textDesc);
	}
    }

    /**
     * ActionListener interface implementation. All fields in this
     * panel will generate this message when changed. We use the
     * internal UI state to save the user choices so we just reset
     * the hasChanged field of our model
     *
     * @param ActionEvent info about the event
     */
    public void actionPerformed(ActionEvent e) 
    {
      try {
          if (e.getSource() == updateButton) {

 	     // Invoke Update Checker 
             String jupdchecker = System.getProperty("java.home") +
                                  File.separator + "bin" +
                                  File.separator + "jucheck.exe";
             Runtime.getRuntime().exec(jupdchecker);
         } else if (e.getSource() == advancedButton) {
             new AdvancedDialog(this, model, mh); 
         } else if (e.getSource() == notify) {
             int index = notify.getSelectedIndex();
	     model.setUpdateNotify(index);
	     setText();
         } else if (e.getSource() == updateCheckBox) {
	     boolean b = updateCheckBox.isSelected();
             model.setUpdateEnabled(b);
	     setText();
         } 
      } catch(Exception exc) {
          DialogFactory.showExceptionDialog(this, exc);
      }
    }

    public String[] getAtComboBoxText() {return atComboBoxText; }
    public String[] getWeekDays() {return weekDays; }

    private MessageHandler mh = new MessageHandler("update");

    private static final String update_image = "sun/plugin/panel/JavaUpgrade48x48.gif";
    private JButton updateButton;
    private JButton advancedButton;
    private JComboBox notify;
    private JCheckBox updateCheckBox;
    private JLabel textDescLabel;
    private JLabel lastRunLabel;
    private String textDesc;
    private DateFormat df = new SimpleDateFormat("EEE, dd MMM yyyy hh:mm:ss zzz",Locale.US);
    private String notifyText = mh.getMessage("desc_notify.text");
    private String checkText = mh.getMessage("desc_check.text");
    private String ntfyInstallText = mh.getMessage("notify_install.text");
    private String ntfyDownldText = mh.getMessage("notify_download.text");
    private String[] weekDays ;
    private String[] atComboBoxText;
}
