import React, { Component } from 'react';

import PropTypes from 'prop-types';
import { withStyles } from 'material-ui/styles';
import classNames from 'classnames';
import Drawer from 'material-ui/Drawer';
import AppBar from 'material-ui/AppBar';
import Toolbar from 'material-ui/Toolbar';
import List from 'material-ui/List';
import Typography from 'material-ui/Typography';
import Divider from 'material-ui/Divider';
import IconButton from 'material-ui/IconButton';
import Menu, { MenuItem } from 'material-ui/Menu';
import IconAccount from 'material-ui-icons/AccountCircle';
import MenuIcon from 'material-ui-icons/Menu';
import ChevronLeftIcon from 'material-ui-icons/ChevronLeft';
import { ListItem, ListItemIcon, ListItemText } from 'material-ui/List';
import IconDashboard from 'material-ui-icons/Dashboard';
import IconServer from 'material-ui-icons/Dns';
import IconBackup from 'material-ui-icons/Restore';
import IconMaps from 'material-ui-icons/Map';
import IconConsole from 'material-ui-icons/CallToAction';
import IconCode from 'material-ui-icons/Code';
import PageDashboard from './pages/dashboard';
import PageServer from './pages/server';
import PageBackup from './pages/backup';
import PageMaps from './pages/maps';
import PageConsole from './pages/console';

import './App.css';

const drawerWidth = 240;

const styles = theme => ({
  root: {
    flexGrow: 1,
    zIndex: 1,
    overflow: 'hidden',
    position: 'relative',
    display: 'flex',
  },
  appBar: {
    zIndex: theme.zIndex.drawer + 1,
    transition: theme.transitions.create(['width', 'margin'], {
      easing: theme.transitions.easing.sharp,
      duration: theme.transitions.duration.leavingScreen,
    }),
  },
  appBarShift: {
    marginLeft: drawerWidth,
    width: `calc(100% - ${drawerWidth}px)`,
    transition: theme.transitions.create(['width', 'margin'], {
      easing: theme.transitions.easing.sharp,
      duration: theme.transitions.duration.enteringScreen,
    }),
  },
  menuButton: {
    marginLeft: -12,
    marginRight: 36,
  },
  flex: {
    flex: 1,
  },
  hide: {
    display: 'none',
  },
  drawerPaper: {
    position: 'relative',
    width: drawerWidth,
    transition: theme.transitions.create('width', {
      easing: theme.transitions.easing.sharp,
      duration: theme.transitions.duration.enteringScreen,
    }),
    'white-space': 'nowrap',
  },
  drawerPaperClose: {
    overflowX: 'hidden',
    transition: theme.transitions.create('width', {
      easing: theme.transitions.easing.sharp,
      duration: theme.transitions.duration.leavingScreen,
    }),
    width: theme.spacing.unit * 7,
    [theme.breakpoints.up('sm')]: {
      width: theme.spacing.unit * 9,
    },
  },
  toolbar: {
    display: 'flex',
    alignItems: 'center',
    justifyContent: 'flex-end',
    padding: '0 8px',
    ...theme.mixins.toolbar,
  },
  content: {
    flexGrow: 1,
    backgroundColor: theme.palette.background.default,
    padding: theme.spacing.unit * 3,
  },
});

const pages = [
  {
    name: "dashboard",
    title: "仪表盘",
    icon: <IconDashboard />,
    body: <PageDashboard />,
  },
  {
    name: "server",
    title: "服务器控制",
    icon: <IconServer />,
    body: <PageServer />,
  },
  {
    name: "backup",
    title: "备份管理",
    icon: <IconBackup />,
    body: <PageBackup />,
  },
  {
    name: "maps",
    title: "世界地图",
    icon: <IconMaps />,
    body: <PageMaps />,
  },
  {
    name: "console",
    title: "控制台",
    icon: <IconConsole />,
    body: <PageConsole />,
  },
]

class App extends Component {
  state = {
    open: false,
    page: pages[0],
    anchorEl: null,
  };

  handleDrawerOpen = () => {
    this.setState({ open: true });
  };

  handleDrawerClose = () => {
    this.setState({ open: false });
  };

  handleConnectionMenu = event => {
    this.setState({ anchorEl: event.currentTarget });
  };

  handleConnectionMenuClose = () => {
    this.setState({ anchorEl: null });
  };

  render() {
    const { classes } = this.props;

    return (
      <div className={classes.root}>
        <AppBar
          position="absolute"
          className={classNames(classes.appBar, this.state.open && classes.appBarShift)}
        >
          <Toolbar>
            <IconButton
              color="inherit"
              aria-label="open drawer"
              onClick={this.handleDrawerOpen}
              className={classNames(classes.menuButton, this.state.open && classes.hide)}
            >
              <MenuIcon />
            </IconButton>
            <Typography variant="title" color="inherit" noWrap className={classes.flex}>
              ServMC - {this.state.page.title}
            </Typography>
            <IconButton
              aria-owns={Boolean(this.state.anchorEl) ? 'menu-appbar' : null}
              aria-haspopup="true"
              onClick={this.handleConnectionMenu}
              color="inherit"
            >
              <IconAccount />
            </IconButton>
            <Menu
              id="menu-appbar"
              anchorEl={this.state.anchorEl}
              anchorOrigin={{
                vertical: 'top',
                horizontal: 'right',
              }}
              transformOrigin={{
                vertical: 'top',
                horizontal: 'right',
              }}
              open={Boolean(this.state.anchorEl)}
              onClose={this.handleConnectionMenuClose}
            >
              <MenuItem onClick={() => {
                this.handleConnectionMenuClose()
              }}>……</MenuItem>
              <MenuItem onClick={() => {
                this.handleConnectionMenuClose()
              }}>……</MenuItem>
            </Menu>
          </Toolbar>
        </AppBar>
        <Drawer
          variant="permanent"
          classes={{
            paper: classNames(classes.drawerPaper, !this.state.open && classes.drawerPaperClose),
          }}
          open={this.state.open}
        >
          <div className={classes.toolbar}>
            <IconButton onClick={this.handleDrawerClose}>
              <ChevronLeftIcon />
            </IconButton>
          </div>
          <Divider />
          <List>
            {pages.map((item, index) => {
              return (
                <ListItem key={index} button onClick={() => { this.setState({ page: item }) }}>
                  <ListItemIcon>
                    {item.icon}
                  </ListItemIcon>
                  <ListItemText primary={item.title} />
                </ListItem>
              );
            })}
          </List>
          <Divider />
          <List>
            <ListItem button onClick={() => { window.open("https://github.com/zhiyb/servmc/") }}>
              <ListItemIcon>
                <IconCode />
              </ListItemIcon>
              <ListItemText primary="访问ServMC项目" />
            </ListItem>
          </List>
        </Drawer>
        <main className={classes.content}>
          <div className={classes.toolbar} />
          {this.state.page.body}
        </main>
      </div>
    );
  }

  static propTypes = {
    classes: PropTypes.object.isRequired,
  };
}


// export default App;
export default withStyles(styles, { withTheme: true })(App);
