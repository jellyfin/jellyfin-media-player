// Enhanced SSO Plugin for Jellyfin Media Player with Deeplink Support
// Configuration - Update this URL to match your SSO endpoint
const SSO_AUTH_URL = 'https://jellyfin.xxx.yyy/sso/OID/start/';

// Deeplink configuration
const DEEPLINK_SCHEMES = ['jellyfin', 'jmp'];
const DEEPLINK_SSO_ACTION = 'sso';

(function waitForBody() {
  if (!document.body) return setTimeout(waitForBody, 100);

  function isLoginPage() {
    const hash = location.hash.toLowerCase();
    const pathname = location.pathname.toLowerCase();

    const hasLoginUrl = (
      hash === '' ||
      hash === '#/' ||
      hash === '#/home' ||
      hash === '#/login' ||
      hash.startsWith('#/login') ||
      pathname.includes('/login')
    );

    const hasLoginElements = (
      document.querySelector('input[type="password"]') !== null ||
      document.querySelector('.loginPage') !== null ||
      document.querySelector('#txtUserName') !== null
    );

    return hasLoginUrl || hasLoginElements;
  }

  function shouldExcludePage() {
    const hash = location.hash.toLowerCase();
    const excludePatterns = [
      '#/dashboard',
      '#/home.html',
      '#/movies',
      '#/tv',
      '#/music',
      '#/livetv',
      '#/search',
      '#/settings',
      '#/wizardstart',
      '#/wizardfinish',
      '#/mypreferencesmenu',
      '#/userprofile'
    ];
    return excludePatterns.some(pattern => hash.startsWith(pattern));
  }

  // Enhanced desktop app detection
  function isDesktopApp() {
    return !!(
      window.NativeShell || 
      window.require || 
      navigator.userAgent.includes('Electron') ||
      window.jmpInfo || // Jellyfin Media Player specific
      window.api ||
      // Check for deeplink handler component
      (window.components && window.components.deeplink)
    );
  }

  // Deeplink SSO integration for Jellyfin Media Player
  function isDeeplinkCapable() {
    return !!(window.components && window.components.deeplink);
  }

  // Enhanced device ID initialization
  function oAuthInitDeviceId() {
    try {
      if (!localStorage.getItem('_deviceId2')) {
        let deviceId = null;
        
        // Try multiple sources for device ID
        if (window.NativeShell?.AppHost?.deviceId) {
          deviceId = window.NativeShell.AppHost.deviceId();
        } else if (window.jmpInfo?.deviceId) {
          deviceId = window.jmpInfo.deviceId;
        } else if (window.api?.system?.deviceId) {
          // Try async device ID if available
          window.api.system.deviceId((id) => {
            if (id && !localStorage.getItem('_deviceId2')) {
              localStorage.setItem('_deviceId2', id);
            }
          });
        }
        
        if (deviceId) {
          localStorage.setItem('_deviceId2', deviceId);
        }
      }
    } catch (e) {
      console.warn('Could not initialize device ID:', e);
    }
  }

  // Generate SSO deeplink URL
  function generateSsoDeeplink(serverUrl, returnUrl) {
    const baseUrl = `jellyfin://sso?server=${encodeURIComponent(serverUrl)}`;
    if (returnUrl) {
      return `${baseUrl}&return_url=${encodeURIComponent(returnUrl)}`;
    }
    return baseUrl;
  }

  // Handle SSO via deeplink (for external app integration)
  function handleSsoViaDeeplink(serverUrl, returnUrl) {
    return new Promise((resolve, reject) => {
      if (!isDeeplinkCapable()) {
        reject(new Error('Deeplink functionality not available'));
        return;
      }

      try {
        const deeplinkUrl = generateSsoDeeplink(serverUrl, returnUrl || window.location.href);
        console.log('Processing SSO via deeplink:', deeplinkUrl);
        
        // Use the deeplink handler component
        const success = window.components.deeplink.processDeepLink(deeplinkUrl);
        
        if (success) {
          // Set up listener for SSO completion
          const checkForAuth = setInterval(() => {
            // Check for authentication success (customize based on your Jellyfin setup)
            const isAuthenticated = checkAuthenticationStatus();
            if (isAuthenticated) {
              clearInterval(checkForAuth);
              resolve({ success: true, method: 'deeplink' });
            }
          }, 1000);

          // Timeout after 5 minutes
          setTimeout(() => {
            clearInterval(checkForAuth);
            reject(new Error('SSO authentication timeout via deeplink'));
          }, 300000);
        } else {
          reject(new Error('Failed to process SSO deeplink'));
        }
      } catch (error) {
        reject(error);
      }
    });
  }

  // Check current authentication status
  function checkAuthenticationStatus() {
    // Customize this based on your Jellyfin authentication indicators
    try {
      // Check for auth tokens, logged-in user, or other indicators
      const hasAuthToken = !!localStorage.getItem('jellyfin_credentials') ||
                          !!sessionStorage.getItem('jellyfin_credentials') ||
                          !!document.cookie.match(/X-Jellyfin-Token|jellyfin[_-]?auth/i);
      
      const hasUserData = !!localStorage.getItem('jellyfin_userId') ||
                         !document.querySelector('.loginPage, #loginPage');
      
      return hasAuthToken || hasUserData;
    } catch (e) {
      console.warn('Error checking authentication status:', e);
      return false;
    }
  }

  // Enhanced SSO overlay with better error handling and UX
  function createSSOOverlay() {
    return new Promise((resolve, reject) => {
      // Create overlay with better styling
      const overlay = document.createElement('div');
      overlay.id = 'sso-overlay';
      overlay.style.cssText = `
        position: fixed;
        top: 0;
        left: 0;
        width: 100%;
        height: 100%;
        background: rgba(0, 0, 0, 0.85);
        z-index: 10000;
        display: flex;
        align-items: center;
        justify-content: center;
        backdrop-filter: blur(5px);
      `;

      // Create modal with responsive design
      const modal = document.createElement('div');
      modal.style.cssText = `
        background: #181818;
        border-radius: 12px;
        width: min(90vw, 900px);
        height: min(85vh, 800px);
        position: relative;
        box-shadow: 0 8px 32px rgba(0, 0, 0, 0.7);
        display: flex;
        flex-direction: column;
        border: 1px solid #333;
      `;

      // Enhanced header
      const header = document.createElement('div');
      header.style.cssText = `
        padding: 20px;
        border-bottom: 1px solid #333;
        display: flex;
        justify-content: space-between;
        align-items: center;
        color: white;
        font-size: 18px;
        font-weight: bold;
        background: linear-gradient(135deg, #2d3748 0%, #1a202c 100%);
        border-radius: 12px 12px 0 0;
      `;

      // Progress indicator
      const progressBar = document.createElement('div');
      progressBar.style.cssText = `
        position: absolute;
        top: 0;
        left: 0;
        width: 0%;
        height: 3px;
        background: linear-gradient(90deg, #4299e1, #3182ce);
        border-radius: 12px 0 0 0;
        transition: width 0.3s ease;
        opacity: 0;
      `;

      const showProgress = (show) => {
        progressBar.style.opacity = show ? '1' : '0';
        if (show) {
          progressBar.style.width = '100%';
        }
      };

      header.innerHTML = `
        <span>🔐 SSO Login ${isDeeplinkCapable() ? '(Deeplink Enhanced)' : ''}</span>
        <button id="sso-close" style="background: none; border: none; color: white; font-size: 24px; cursor: pointer; padding: 8px; border-radius: 6px; transition: background-color 0.2s;" 
                onmouseover="this.style.backgroundColor='rgba(255,255,255,0.1)'" 
                onmouseout="this.style.backgroundColor='transparent'">&times;</button>
      `;

      // Loading indicator
      const loadingIndicator = document.createElement('div');
      loadingIndicator.style.cssText = `
        position: absolute;
        top: 50%;
        left: 50%;
        transform: translate(-50%, -50%);
        color: white;
        font-size: 16px;
        display: flex;
        align-items: center;
        gap: 10px;
        z-index: 1;
      `;
      loadingIndicator.innerHTML = `
        <div style="width: 20px; height: 20px; border: 2px solid #333; border-top: 2px solid #4299e1; border-radius: 50%; animation: spin 1s linear infinite;"></div>
        Loading SSO...
      `;

      // Add CSS animation for spinner
      if (!document.getElementById('sso-spinner-style')) {
        const style = document.createElement('style');
        style.id = 'sso-spinner-style';
        style.textContent = `
          @keyframes spin {
            0% { transform: rotate(0deg); }
            100% { transform: rotate(360deg); }
          }
        `;
        document.head.appendChild(style);
      }

      // Create iframe with better error handling
      const iframe = document.createElement('iframe');
      iframe.style.cssText = `
        flex: 1;
        border: none;
        width: 100%;
        border-radius: 0 0 12px 12px;
        background: white;
      `;

      // Iframe loading handler
      iframe.onload = () => {
        loadingIndicator.style.display = 'none';
        showProgress(false);
      };

      iframe.onerror = () => {
        loadingIndicator.innerHTML = '<div style="color: #ff6b6b;">Failed to load SSO page</div>';
      };

      // Assemble modal
      modal.appendChild(progressBar);
      modal.appendChild(header);
      modal.appendChild(loadingIndicator);
      modal.appendChild(iframe);
      overlay.appendChild(modal);

      // Add to document
      document.body.appendChild(overlay);

      // Show progress and load iframe
      showProgress(true);
      iframe.src = SSO_AUTH_URL;

      // Enhanced close button handler
      const closeBtn = header.querySelector('#sso-close');
      closeBtn.addEventListener('click', () => {
        cleanup();
        reject(new Error('Authentication cancelled'));
      });

      // Click outside to close (with confirmation)
      overlay.addEventListener('click', (e) => {
        if (e.target === overlay) {
          if (confirm('Are you sure you want to cancel the SSO login?')) {
            cleanup();
            reject(new Error('Authentication cancelled'));
          }
        }
      });

      // Cleanup function
      const cleanup = () => {
        if (document.body.contains(overlay)) {
          document.body.removeChild(overlay);
        }
        clearInterval(checkIframe);
        window.removeEventListener('message', messageHandler);
      };

      // Enhanced iframe monitoring with better URL detection
      let checkCount = 0;
      const maxChecks = 300; // 5 minutes
      let lastUrl = '';

      const checkIframe = setInterval(() => {
        checkCount++;
        
        try {
          const iframeUrl = iframe.contentWindow.location.href;
          
          // Log URL changes for debugging
          if (iframeUrl !== lastUrl) {
            console.log('SSO iframe navigated to:', iframeUrl);
            lastUrl = iframeUrl;
          }
          
          // Check if we're back at Jellyfin (more comprehensive patterns)
          const jellyfinPatterns = [
            '/web/',
            '#/',
            '/dashboard',
            'index.html',
            // Add your specific Jellyfin URLs here
          ];
          
          const isBackAtJellyfin = jellyfinPatterns.some(pattern => 
            iframeUrl.includes(pattern)
          );
          
          if (isBackAtJellyfin && !iframeUrl.includes('sso')) {
            console.log('SSO authentication successful, returned to Jellyfin');
            cleanup();
            resolve({ success: true, url: iframeUrl });
            return;
          }
          
        } catch (e) {
          // Cross-origin errors are expected during OAuth flow
          // But we can still detect when the iframe has finished loading
        }
        
        // Timeout check
        if (checkCount >= maxChecks) {
          cleanup();
          reject(new Error('Authentication timeout (5 minutes exceeded)'));
        }
      }, 1000);

      // Enhanced postMessage handler
      const messageHandler = (event) => {
        // Verify origin for security
        try {
          const ssoOrigin = new URL(SSO_AUTH_URL).origin;
          if (event.origin === ssoOrigin || event.origin === window.location.origin) {
            if (event.data.type === 'sso_success') {
              console.log('Received SSO success message:', event.data);
              cleanup();
              resolve(event.data);
            } else if (event.data.type === 'sso_error') {
              console.error('Received SSO error message:', event.data);
              cleanup();
              reject(new Error(event.data.message || 'SSO authentication failed'));
            } else if (event.data.type === 'sso_redirect') {
              console.log('Received SSO redirect message:', event.data);
              // Handle redirect if your SSO provider supports it
              iframe.src = event.data.url;
            }
          }
        } catch (e) {
          console.warn('Invalid message received:', e);
        }
      };
      window.addEventListener('message', messageHandler);

      // Keyboard shortcuts
      const keyHandler = (e) => {
        if (e.key === 'Escape') {
          if (confirm('Are you sure you want to cancel the SSO login?')) {
            cleanup();
            reject(new Error('Authentication cancelled'));
          }
        }
      };
      document.addEventListener('keydown', keyHandler);

      // Add keyHandler to cleanup
      const originalCleanup = cleanup;
      cleanup = () => {
        document.removeEventListener('keydown', keyHandler);
        originalCleanup();
      };
    });
  }

  // Enhanced SSO authentication with deeplink and overlay support
  async function handleSSOAuth() {
    try {
      oAuthInitDeviceId();
      
      const currentUrl = window.location.href;
      const serverUrl = new URL(currentUrl).origin;

      if (isDesktopApp()) {
        console.log('Jellyfin Media Player detected');
        
        // Try deeplink method first if available
        if (isDeeplinkCapable()) {
          console.log('Using deeplink-enhanced SSO flow');
          try {
            const deeplinkResult = await handleSsoViaDeeplink(serverUrl, currentUrl);
            console.log('Deeplink SSO completed:', deeplinkResult);
            return;
          } catch (deeplinkError) {
            console.warn('Deeplink SSO failed, falling back to iframe:', deeplinkError);
          }
        }
        
        // Fallback to iframe overlay
        console.log('Using iframe overlay SSO flow');
        const result = await createSSOOverlay();
        console.log('SSO authentication completed:', result);
        
        // Give a moment for any final redirects/cookies to process
        await new Promise(resolve => setTimeout(resolve, 1500));
        
        // Refresh the main window
        console.log('Refreshing main window...');
        window.location.reload();
        
      } else {
        console.log('Browser detected, using direct navigation');
        // For browsers, navigate in same tab
        window.location.href = SSO_AUTH_URL;
      }
      
    } catch (error) {
      console.error('SSO authentication failed:', error);
      
      // Enhanced error reporting
      const errorMsg = error.message || 'SSO authentication failed';
      
      // Try multiple notification methods
      if (typeof window.Dashboard?.alert === 'function') {
        window.Dashboard.alert({
          title: 'SSO Error',
          message: errorMsg
        });
      } else if (window.require) {
        // Try electron dialog
        try {
          const { dialog } = window.require('electron').remote || window.require('@electron/remote');
          dialog.showErrorBox('SSO Error', errorMsg);
        } catch {
          alert(`SSO Error: ${errorMsg}`);
        }
      } else {
        // Fallback to styled alert
        const alertDiv = document.createElement('div');
        alertDiv.style.cssText = `
          position: fixed;
          top: 20px;
          right: 20px;
          background: #ff6b6b;
          color: white;
          padding: 15px 20px;
          border-radius: 8px;
          box-shadow: 0 4px 12px rgba(0,0,0,0.3);
          z-index: 10001;
          max-width: 300px;
          font-family: Arial, sans-serif;
        `;
        alertDiv.innerHTML = `
          <div style="font-weight: bold; margin-bottom: 5px;">🚫 SSO Authentication Error</div>
          <div style="font-size: 14px;">${errorMsg}</div>
        `;
        document.body.appendChild(alertDiv);
        
        setTimeout(() => {
          if (document.body.contains(alertDiv)) {
            document.body.removeChild(alertDiv);
          }
        }, 5000);
      }
    }
  }

  // Listen for deeplink-triggered SSO events
  function setupDeeplinkSsoListener() {
    if (isDeeplinkCapable()) {
      // Connect to the deeplink component's SSO signal
      try {
        // Listen for SSO requests via deeplink handler
        if (window.components.deeplink.ssoRequested) {
          window.components.deeplink.ssoRequested.connect((server, returnUrl) => {
            console.log('SSO requested via deeplink:', server, returnUrl);
            handleSSOAuth().catch(error => {
              console.error('Deeplink-triggered SSO failed:', error);
            });
          });
        }
      } catch (e) {
        console.warn('Could not set up deeplink SSO listener:', e);
      }
    }
  }

  // Enhanced button insertion with deeplink information
  function insertSSOButton() {
    if (!isLoginPage() || shouldExcludePage()) return;

    const loginContainer =
      document.querySelector('.readOnlyContent') ||
      document.querySelector('form')?.parentNode ||
      document.querySelector('.loginPage') ||
      document.querySelector('#loginPage');

    if (!loginContainer || document.querySelector('#custom-sso-button')) return;

    // Create the SSO button element with enhanced styling
    const button = document.createElement('button');
    button.id = 'custom-sso-button';
    button.type = 'button';
    button.className = 'raised block emby-button button-submit';

    // Enhanced button styling
    button.style.cssText = `
      display: flex;
      align-items: center;
      justify-content: center;
      gap: 12px;
      padding: 14px 24px;
      font-size: 16px;
      font-weight: 600;
      background: linear-gradient(135deg, #3949ab 0%, #303f9f 100%);
      color: #fff;
      margin-top: 16px;
      border: none;
      cursor: pointer;
      border-radius: 6px;
      transition: all 0.3s ease;
      box-shadow: 0 2px 8px rgba(57, 73, 171, 0.3);
      position: relative;
      overflow: hidden;
    `;

    // Add hover and focus effects
    button.addEventListener('mouseenter', () => {
      if (!button.disabled) {
        button.style.transform = 'translateY(-1px)';
        button.style.boxShadow = '0 4px 16px rgba(57, 73, 171, 0.4)';
      }
    });

    button.addEventListener('mouseleave', () => {
      if (!button.disabled) {
        button.style.transform = 'translateY(0)';
        button.style.boxShadow = '0 2px 8px rgba(57, 73, 171, 0.3)';
      }
    });

    // Button content with icon
    const icon = document.createElement('span');
    icon.className = 'material-icons';
    icon.style.fontSize = '20px';
    icon.textContent = 'shield';

    const text = document.createElement('span');
    const deeplinkStatus = isDeeplinkCapable() ? ' (Enhanced)' : '';
    text.textContent = `Login with SSO${deeplinkStatus}`;

    button.appendChild(icon);
    button.appendChild(text);

    // Enhanced click handler
    button.addEventListener('click', async (e) => {
      e.preventDefault();
      e.stopPropagation();
      
      // Visual feedback
      button.disabled = true;
      button.style.opacity = '0.7';
      button.style.transform = 'scale(0.98)';
      button.style.cursor = 'not-allowed';
      
      const originalText = text.textContent;
      
      try {
        text.textContent = 'Opening SSO...';
        icon.style.animation = 'spin 2s linear infinite';
        
        await handleSSOAuth();
      } catch (error) {
        // Reset button state on error
        text.textContent = originalText;
        icon.style.animation = '';
        button.disabled = false;
        button.style.opacity = '1';
        button.style.transform = 'scale(1)';
        button.style.cursor = 'pointer';
      }
    });

    // Insert button in the most appropriate location
    const pwInput = document.querySelector('input[type="password"]');
    if (pwInput?.parentNode) {
      // Insert after password field
      const nextSibling = pwInput.nextSibling;
      if (nextSibling?.nextSibling) {
        pwInput.parentNode.insertBefore(button, nextSibling.nextSibling);
      } else {
        pwInput.parentNode.appendChild(button);
      }
    } else {
      loginContainer.appendChild(button);
    }
  }

  // Enhanced initialization and monitoring
  function initializeSSO() {
    if (isLoginPage() && !shouldExcludePage()) {
      console.log('SSO: Login page detected, initializing...');
      setTimeout(insertSSOButton, 500);
    }
    
    // Set up deeplink SSO listener
    setupDeeplinkSsoListener();
  }

  // Initial setup
  initializeSSO();

  // Enhanced SPA monitoring
  const observer = new MutationObserver((mutations) => {
    let shouldCheck = false;
    mutations.forEach((mutation) => {
      if (mutation.type === 'childList' && mutation.addedNodes.length > 0) {
        shouldCheck = true;
      }
    });
    
    if (shouldCheck && isLoginPage() && !shouldExcludePage() && !document.querySelector('#custom-sso-button')) {
      setTimeout(insertSSOButton, 100);
    }
  });
  
  observer.observe(document.body, { 
    childList: true, 
    subtree: true,
    attributes: false,
    characterData: false
  });

  // Enhanced hash change monitoring
  window.addEventListener('hashchange', () => {
    console.log('SSO: Hash changed to:', window.location.hash);
    setTimeout(() => {
      if (isLoginPage() && !shouldExcludePage()) {
        initializeSSO();
      }
    }, 300);
  });

  // Popstate monitoring for better SPA support
  window.addEventListener('popstate', () => {
    console.log('SSO: Popstate event detected');
    setTimeout(() => {
      if (isLoginPage() && !shouldExcludePage()) {
        initializeSSO();
      }
    }, 300);
  });

  // Add global function for external deeplink integration
  window.handleSsoViaDeeplink = handleSsoViaDeeplink;
  window.generateSsoDeeplink = generateSsoDeeplink;

  console.log('SSO Plugin initialized for Jellyfin Media Player with Deeplink Support');
})();